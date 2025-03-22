import logging
from queue import Queue
from typing import List

import asyncio
import cv2
import numpy as np


log = logging.getLogger()


class VideoCapture:
    def __init__(self, source: int | str, stop_condition: asyncio.Event):
        """
        Initialize camera for capturing live frames.

        Args:
            source (int | str): Camera index or filename.

        Raises:
            AssertionError: Invalid camera source.
        """
        self.stop_condition = stop_condition
        self.cap = cv2.VideoCapture(source)
        assert self.cap.isOpened(), "Error: Could not open video source"
        log.info("Video capture initialized")

    def __del__(self):
        """
        Release camera resources.
        """
        if hasattr(self, "cap"):
            self.cap.release()
        log.info("VideoCapture resources released")

    async def get_frame(self, queues: List[Queue[np.ndarray]]):
        """
        Capture frames from the camera and enqueue them
        to the input list of queues.

        Args:
            queues (List[Queue[np.ndarray]]): List of queues waiting for
                                              camera frames to be enqueued.
        """
        log.info("Starting to gather video frames")
        skip_frame = 4
        frame_count = 0

        while not self.stop_condition.is_set():
            ret, frame = self.cap.read()
            if not ret:
                log.warning("Failed to capture frame")
                continue

            frame_count += 1
            if frame_count % skip_frame != 0:  # skip frames
                continue

            for q in queues:
                flipped_frame = np.fliplr(frame)
                q.put_nowait(flipped_frame)
            await asyncio.sleep(0.01)
