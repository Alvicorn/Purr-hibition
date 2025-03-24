# References: https://medium.com/@tauseefahmad12/object-detection-using-mobilenet-ssd-e75b177567ee

from queue import Queue
import signal

import asyncio

from log_config import serial_logger
from object_detector import ObjDetector
from shared_mem_manager import SharedMemManager
from video_captor import VideoCaptor
from video_streamer import VideoStreamer


CAMERA_PATH = "/dev/video3"

STOP_EVENT = asyncio.Event()

SHARED_MEM_SIZE = 4
SHARED_MEM_NAME = "/dev/shm/byai_cam_cat_detected"


log = serial_logger()


async def main():
    cap = VideoCaptor(CAMERA_PATH, STOP_EVENT)

    object_class_id = 8  # 15 is a human; 8 is a cat
    detector = ObjDetector(
        object_class_id,
        STOP_EVENT,
        SharedMemManager(SHARED_MEM_NAME, SHARED_MEM_SIZE),
        confidence=0.5,
    )

    streamer = VideoStreamer(
        "http://192.168.7.1:8080/video-feed", STOP_EVENT, reconnect_timeout=5
    )

    detector_queue = Queue()
    streamer_queue = Queue()

    tasks = [
        asyncio.create_task(cap.get_frame([detector_queue, streamer_queue])),
        asyncio.create_task(detector.process_queue(detector_queue)),
        asyncio.create_task(streamer.stream_video(streamer_queue)),
    ]

    # signal handling for graceful shutdown
    def signal_handler(sig, frame):
        log.info("\nKeyboardInterrupt received. Stopping all tasks.")
        STOP_EVENT.set()
        for task in tasks:
            task.cancel()

    signal.signal(signal.SIGINT, signal_handler)

    # wait for tasks to complete
    await asyncio.gather(*tasks, return_exceptions=True)


if __name__ == "__main__":
    asyncio.run(main())
