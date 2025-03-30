from enum import Enum
import logging

import asyncio

from shared_mem_manager import SharedMemManager


log = logging.getLogger(__name__)


class Command(Enum):
    NO_ACTION = 0  # all other cases
    KILL = 1  # terminate the BYAI Camera
    START = 2  # start up the BYAI Camera
    STOP = 3  # stop the BYAI Camera (not terminating it)


class BYAICameraState(Enum):
    NO_STATE = 0  # all other cases
    KILLED = 1  # BYAI Camera is marked to be terminated
    SLEEPING = 2  # BYAI Camera has stopped processing
    RUNNING = 3  # BYAI Camera has started processing


class CameraCoordinator:
    def __init__(
        self,
        host_to_target_commands: SharedMemManager,
        byai_camera_state: SharedMemManager,
    ):
        self.host_to_target_commands = host_to_target_commands
        self.byai_camera_state = byai_camera_state
        self.current_state = BYAICameraState.SLEEPING

    async def get_command(self) -> Command:
        try:
            if self.current_state == BYAICameraState.KILLED:
                log.warning("BYAI Camera has been set to shutdown")
                return Command.KILL

            value = await self.host_to_target_commands.read()
            try:
                return Command(value)
            except ValueError:
                return Command.NO_ACTION
        except asyncio.CancelledError:
            return Command.NO_ACTION

    async def set_state(self, state: BYAICameraState) -> None:
        if self.current_state == BYAICameraState.KILLED:
            log.warning("BYAI Camera has already been set to shutdown")
        elif state != BYAICameraState.NO_STATE:
            self.byai_camera_state.write(state.value)
            self.current_state = state
