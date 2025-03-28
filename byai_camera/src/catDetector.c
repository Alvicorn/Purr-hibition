#include "byai_camera/catDetector.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hal/sharedMemory.h"

#define SHARED_MEM_NAME "/byai_cam_cat_detected"  // Shared memory object name

static bool is_initialized = false;
static SharedMemory_t shm;

void CatDetector_init(void) {
  assert(!is_initialized);
  shm = SharedMemory_init(SHARED_MEM_NAME);
  is_initialized = true;
}  // end of CatDetector_init()

void CatDetector_cleanup(void) {
  assert(is_initialized);
  SharedMemory_cleanup(&shm);
  is_initialized = false;
}  // end of CatDetector_cleanup()

bool CatDetector_detected(void) {
  assert(is_initialized);
  return SharedMemory_read(&shm) == 1;  // 1 for cat detected
}  // end of CatDetector_detected()
