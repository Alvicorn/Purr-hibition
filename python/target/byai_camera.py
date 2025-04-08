# References: https://medium.com/@tauseefahmad12/object-detection-using-mobilenet-ssd-e75b177567ee

from dataclasses import dataclass, field
import logging
from queue import Queue
import signal
from typing import Callable, List, Optional, Tuple

import asyncio

from camera_coordinator import BYAICameraState, Command, CameraCoordinator
from log_config import serial_logger
from object_detector import ObjDetector
from shared_mem_manager import SharedMemManager
from video_captor import VideoCaptor
from video_streamer import VideoStreamer


SHARED_MEM_SIZE = 4
SHARED_MEM_PATH = "/dev/shm"
SHARED_MEM_CAT_DETECTED = f"{SHARED_MEM_PATH}/byai_cam_cat_detected"
SHARED_MEM_COMMANDS = f"{SHARED_MEM_PATH}/byai_cam_commands"
SHARED_MEM_BYAI_CAM_STATE = f"{SHARED_MEM_PATH}/byai_cam_state"


log = serial_logger(logging.INFO)


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

    def cancel_task(self) -> None:
        """
        Cancel the asyncio.Task is it exists.
        """
        if self.task is not None:
            self.task.cancel()
            self.task = None


async def run_tasks(
    tasks: List[Task], stop_event: asyncio.Event, kill_event: asyncio.Event
):
    """
    Run the tasks required for the camera to function.

    Args:
        tasks (Task): List of Tasks objects.
        stop_event (asyncio.Event): Stop event flag shared across async tasks.
        kill_event (asyncio.Event): Kill event flag used to stop the run_task co-routine.
    """

    coordinator = CameraCoordinator(
        SharedMemManager(SHARED_MEM_COMMANDS, SHARED_MEM_SIZE),
        SharedMemManager(SHARED_MEM_BYAI_CAM_STATE, SHARED_MEM_SIZE),
    )

    try:
        await coordinator.set_state(BYAICameraState.SLEEPING)

        while not kill_event.is_set():
            await asyncio.sleep(0.1)
            command = await coordinator.get_command()

            if (
                command == Command.START
                and coordinator.current_state != BYAICameraState.RUNNING
            ):
                log.info("starting tasks")
                if stop_event.is_set():
                    stop_event.clear()

                for task in tasks:
                    task.start_task()

                await coordinator.set_state(BYAICameraState.RUNNING)
                await asyncio.sleep(1)

            elif (
                command == Command.STOP
                and coordinator.current_state != BYAICameraState.SLEEPING
                and not stop_event.is_set()
            ):
                stop_event.set()
                log.info("stopping all tasks")
                for task in tasks:
                    task.cancel_task()
                await coordinator.set_state(BYAICameraState.SLEEPING)
                await asyncio.sleep(1)

            elif command == Command.KILL:
                log.info("quiting")
                stop_event.set()
                kill_event.set()
                await coordinator.set_state(BYAICameraState.KILLED)
                break

    except asyncio.CancelledError:
        log.info("Terminating BYAI program...")
        stop_event.set()
        kill_event.set()
        await coordinator.set_state(BYAICameraState.KILLED)
        raise


async def main():
    # known variable values
    stop_event = asyncio.Event()
    kill_event = asyncio.Event()

    OBJECT_CLASS_ID = 8  # 15 is a human; 8 is a cat
    CAMERA_PATH = "/dev/video3"

    # class instantiations for each task
    cap = VideoCaptor(CAMERA_PATH)
    detector = ObjDetector(
        OBJECT_CLASS_ID,
        SharedMemManager(SHARED_MEM_CAT_DETECTED, SHARED_MEM_SIZE),
        confidence=0.9,
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

    # gracefully handle keyboard interupt
    def signal_handler(sig, frame):
        log.info("\nKeyboardInterrupt received. Stopping all tasks.")
        stop_event.set()
        kill_event.set()
        for task in tasks:
            task.cancel_task()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # start listening from inputs and video processing
    try:
        await run_tasks(tasks, stop_event, kill_event)
    except Exception as e:
        log.info(f"Terminating BYAI Camera program: {e}")
    finally:
        stop_event.set()
        kill_event.set()
        await asyncio.gather(
            *(task.task for task in tasks if task.task is not None),
            return_exceptions=True,
        )
        log.info("Clean up complete")


if __name__ == "__main__":
    asyncio.run(main())
