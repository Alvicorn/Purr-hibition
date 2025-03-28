import logging
import mmap
import os

import asyncio

log = logging.getLogger(__name__)


class SharedMemManager:
    def __init__(self, shared_mem_name: str, shared_mem_size: int) -> None:
        self.shared_mem_name = shared_mem_name
        self.shared_mem_size = shared_mem_size

        self.fd = os.open(self.shared_mem_name, os.O_CREAT | os.O_RDWR)
        os.ftruncate(self.fd, self.shared_mem_size)
        self.shared_mem = mmap.mmap(
            self.fd, self.shared_mem_size, access=mmap.ACCESS_WRITE
        )

        self._min_value = -(2 ** (8 * shared_mem_size - 1))
        self._max_value = (2 ** (8 * shared_mem_size - 1)) - 1

        self.write(0)
        log.info(f"SharedMemManager initialized for {shared_mem_name}")

    def __del__(self) -> None:
        if hasattr(self, "shared_mem"):
            self.shared_mem.close()
        if hasattr(self, "fd"):
            os.close(self.fd)
        os.remove(self.shared_mem_name)
        log.info("SharedMemManager resources released")

    def write(self, value: int) -> None:
        if self._min_value <= value <= self._max_value:
            self.shared_mem.seek(0)
            self.shared_mem.write(
                value.to_bytes(self.shared_mem_size, byteorder="little")
            )
        else:
            log.error(
                f"Invalid shared memeory write to {self.shared_mem_name}. Value {value} "
                + f"is not of range. Must be in the range [{self._min_value}, {self._max_value}]"
            )

    async def read(self) -> int:
        value = await asyncio.to_thread(self._read_shared_mem)
        return value

    def _read_shared_mem(self) -> int:
        self.shared_mem.seek(0)
        value_bytes = self.shared_mem.read(self.shared_mem_size)
        return int.from_bytes(value_bytes, byteorder="little")
