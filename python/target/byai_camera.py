# References: https://medium.com/@tauseefahmad12/object-detection-using-mobilenet-ssd-e75b177567ee

from dataclasses import dataclass, field
from queue import Queue
import signal
from typing import Callable, Dict, Optional, Tuple

import asyncio

from log_config import serial_logger
from object_detector import ObjDetector
from shared_mem_manager import SharedMemManager
from video_captor import VideoCaptor
from video_streamer import VideoStreamer


log = serial_logger()


@dataclass
class Task:
    """
    Wrapper class around each asyncio task. Task maintains the
    required parameters and the asyncio.Task itself.
    """

    task_func: Callable
    params: Tuple
    task: Optional[asyncio.Task] = field(default=None, init=False)

    def start_task(self) -> None:
        """
        Create an asyncio.Task is one has not been created already.
        """
        if self.task is None:
            self.task = asyncio.create_task(self.task_func(*self.params))
        else:
            log.error("Task has already started")

    def cancel_task(self) -> None:
        """
        Cancel the asyncio.Task is it exists.
        """
        if self.task is not None:
            self.task.cancel()
            self.task = None


async def user_input(tasks: Task, state: Dict[str, str], stop_event: asyncio.Event):
    while True:
        command = await asyncio.to_thread(
            input, "Enter command (start/stop/status/quit): "
        )
        command = command.strip().lower()

        if command == "start":
            log.info("starting tasks")
            if state.get("status") == "start":
                log.info("Tasks have already started")
                continue

            if stop_event.is_set():
                stop_event.clear()

            for task in tasks:
                task.start_task()

            state["status"] = "start"
            await asyncio.sleep(1)

        elif command == "stop":
            if not stop_event.is_set():
                stop_event.set()
                log.info("stopping all tasks")
                state["status"] = "stop"
            else:
                log.info("No tasks to stop")

        elif command == "status":
            log.info(f"current status: {state['status']}")

        elif command == "quit":
            log.info("quiting")
            stop_event.set()
            break

        else:
            log.warning(
                "Invalid command. Must be one of 'start', 'stop', 'status' or 'quit'"
            )


async def main():
    # known variable values
    stop_event = asyncio.Event()
    OBJECT_CLASS_ID = 8  # 15 is a human; 8 is a cat
    CAMERA_PATH = "/dev/video3"
    SHARED_MEM_SIZE = 4
    SHARED_MEM_NAME = "/dev/shm/byai_cam_cat_detected"

    # class instantiations for each task
    cap = VideoCaptor(CAMERA_PATH)
    detector = ObjDetector(
        OBJECT_CLASS_ID,
        SharedMemManager(SHARED_MEM_NAME, SHARED_MEM_SIZE),
        confidence=0.5,
    )
    streamer = VideoStreamer("http://192.168.7.1:8080/video-feed", reconnect_timeout=5)

    # messaging queue passing frames between tasks for processing
    detector_queue = Queue()
    streamer_queue = Queue()

    # async tasks
    tasks = [
        Task(cap.get_frame, (stop_event, [detector_queue, streamer_queue])),
        Task(detector.process_queue, (stop_event, detector_queue)),
        Task(streamer.stream_video, (stop_event, streamer_queue)),
    ]

    # current system state
    state = {"status": "stopped"}

    # gracefully handle keyboard interupt
    def signal_handler(sig, frame):
        log.info("\nKeyboardInterrupt received. Stopping all tasks.")
        stop_event.set()
        for task in tasks:
            task.cancel_task()
            # task["task"].cancel()

    signal.signal(signal.SIGINT, signal_handler)

    # start listening from inputs and video processing
    await user_input(tasks, state, stop_event)


if __name__ == "__main__":
    asyncio.run(main())
