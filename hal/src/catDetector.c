#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hal/cat_detector.h"

#define SHARED_MEM_SIZE 4  // Size of shared memory (4 bytes)
#define SHARED_MEM_NAME "/byai_cam_cat_detected"  // Shared memory object name
#define SHARED_MEM_COMMANDS = "/byai_cam_commands"
#define SHARED_MEM_BYAI_CAM_STATE = "/byai_cam_state"

static bool is_initialized = false;
static int fd;
static int *shared_mem;

void CatDetector_init(void) {
  assert(!is_initialized);

  // Open the shared memory object
  fd = shm_open(SHARED_MEM_NAME, O_RDONLY, 0666);
  if (fd == -1) {
    printf("shm_open failed");
    exit(1);
  }

  // Memory-map the shared memory object
  shared_mem = (int *)mmap(NULL, SHARED_MEM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (shared_mem == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }
  is_initialized = true;
}  // end of CatDetector_init()

void CatDetector_cleanup(void) {
  assert(is_initialized);

  // Cleanup shared memory
  if (munmap(shared_mem, SHARED_MEM_SIZE) == -1) {
    perror("munmap failed");
    exit(1);
  }
  close(fd);
  is_initialized = false;
}  // end of CatDetector_cleanup()

bool CatDetector_detected(void) {
  int val = *shared_mem;  // 1 for cat detected
  return val == 1;
}  // end of CatDetector_detected()
