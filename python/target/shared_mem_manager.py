import logging
import mmap
import os


log = logging.getLogger(__name__)


class SharedMemManager:
    def __init__(self, shared_mem_name: str, shared_mem_size: int):
        self.shared_mem_name = shared_mem_name
        self.shared_mem_size = shared_mem_size

        self.fd = os.open(self.shared_mem_name, os.O_CREAT | os.O_RDWR)
        os.ftruncate(self.fd, self.shared_mem_size)
        self.shared_mem = mmap.mmap(
            self.fd, self.shared_mem_size, access=mmap.ACCESS_WRITE
        )
        self.write_false()
        log.info("SharedMemManager initialized")

    def write_true(self):
        self._write_to_mem(1)

    def write_false(self):
        self._write_to_mem(0)

    def _write_to_mem(self, value: int):
        self.shared_mem.seek(0)
        self.shared_mem.write(value.to_bytes(self.shared_mem_size, byteorder="little"))

    def __del__(self):
        self.write_false()
        if hasattr(self, "shared_mem"):
            self.shared_mem.close()
        if hasattr(self, "fd"):
            os.close(self.fd)
        os.remove(self.shared_mem_name)
        log.info("SharedMemManager resources released")
