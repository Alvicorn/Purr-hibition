import concurrent.futures
import logging
from queue import Queue
from typing import List

import asyncio
import cv2
import numpy as np

from shared_mem_manager import SharedMemManager


log = logging.getLogger(__name__)


class ObjDetector:
    def __init__(
        self,
        obj_class_id: int,
        shared_mem_manager: SharedMemManager,
        confidence: int = 0.9,
    ):
        self.obj_class_id = obj_class_id
        self.confidence = confidence
        prototxt = "MobileNetSSD/MobileNetSSD_deploy.prototxt"
        caffe_model = "MobileNetSSD/MobileNetSSD_deploy.caffemodel"
        self.net = cv2.dnn.readNetFromCaffe(prototxt, caffe_model)
        self.shared_mem_manager = shared_mem_manager
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=8)
        self.consecutive_detections = 0
        log.info("ObjDetect initialized")

    def __del__(self):
        self.executor.shutdown(wait=True)
        log.info("ObjDetector resources released")

    def detect_batch(self, frames: List[np.ndarray]):
        """
        Analyze a list of frames to identify the target object.
        If an object is found. write 1 to the shared memory. Otherwise,
        write 0.

        Args:
            frames (List[np.ndarray]): List of frames to be processed.
        """
        resized_dim = (160, 160)

        resized_frames = [cv2.resize(frame, resized_dim) for frame in frames]
        blob = cv2.dnn.blobFromImages(
            resized_frames,
            scalefactor=1 / 127.5,
            size=resized_dim,
            mean=(127.5, 127.5, 127.5),
            swapRB=True,
            crop=False,
        )

        self.net.setInput(blob)
        detections = self.net.forward()

        # obj_found = any(
        #     detections[0, 0, i, 2] > self.confidence
        #     and int(detections[0, 0, i, 1]) == self.obj_class_id
        #     for i in range(detections.shape[2])
        # )

        obj_found = False
        for i in range(detections.shape[2]):
            if int(detections[0, 0, i, 1]) == self.obj_class_id:
                log.debug("cat found... checking confidence")
                if detections[0, 0, i, 2] > self.confidence and detections[0, 0, i, 2] <= 1:
                    log.debug(f"cat detected with confidence: {detections[0, 0, i, 2]}")
                    obj_found = True
                    # algorithm to prevent false positive activations
                    if(obj_found):
                        self.consecutive_detections += 1
                        if(self.consecutive_detections > 3):
                            self.shared_mem_manager.write(1)

                        else:
                            self.self.consecutive_detections = 0
                            self.shared_mem_manager.write(0)

        log.debug(f"conclusion --> cat found: {obj_found}")
        self.shared_mem_manager.write(1 if obj_found else 0)

    async def process_queue(self, stop_event: asyncio.Event, queue: Queue[np.ndarray]):
        """
        Process the frames captured from the device's camera
        and prepares for object identification.

        Args:
            stop_event (asyncio.Event): Stop event flag used shared across async tasks.
            queue (Queue[np.ndarray]): Queue of of frames to be analyze.
        """
        frames = []
        while not stop_event.is_set():
            if not queue.empty():
                frames.append(queue.get_nowait())
                batch_size = 16 if queue.qsize() > 30 else 8
                if len(frames) >= batch_size:
                    self.executor.submit(self.detect_batch, frames.copy())
                    frames = []

            await asyncio.sleep(0.01)
