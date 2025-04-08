#include "hal/sharedMemory.h"

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHARED_MEM_SIZE 4

shared_memory_t SharedMemory_init(char* shared_mem_name) {
  shared_memory_t shm;
  shm.shared_mem_name = shared_mem_name;

  shm.fd = shm_open(shared_mem_name, O_CREAT | O_RDWR, 
    S_IRUSR | S_IWUSR | S_IXUSR |  // Owner permissions
    S_IRGRP | S_IWGRP | S_IXGRP |  // Group permissions
    S_IROTH | S_IWOTH | S_IXOTH); // Other permissions
  if (shm.fd == -1) {
    perror("Failed to open shared memory object");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(shm.fd, SHARED_MEM_SIZE) == -1) {
    perror("Failed to set size of shared memory object");
    close(shm.fd);
    exit(EXIT_FAILURE);
  }

  shm.shared_mem = (int*)mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE,
                              MAP_SHARED, shm.fd, 0);
  if (shm.shared_mem == MAP_FAILED) {
    perror("Failed to map shared memory");
    close(shm.fd);
    exit(EXIT_FAILURE);
  }
  return shm;
}

void SharedMemory_cleanup(shared_memory_t* shm) {
  if (munmap(shm->shared_mem, SHARED_MEM_SIZE) == -1) {
    perror("Failed to unlink the shared memory object");
  }
  close(shm->fd);
}

int SharedMemory_read(shared_memory_t* shm) { return shm->shared_mem[0]; }

void SharedMemory_write(shared_memory_t* shm, int value) {
  shm->shared_mem[0] = value;
}
