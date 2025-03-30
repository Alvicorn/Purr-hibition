#include "byai_camera/catDetector.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "byai_camera/cameraController.h"
#include "hal/motionSensor.h"
#include "hal/sharedMemory.h"

#define SHARED_MEM_NAME "/byai_cam_cat_detected"  // Shared memory object name

static uint16_t timeout;  // timeout in seconds
static atomic_bool cat_detected = ATOMIC_VAR_INIT(false);

static bool is_initialized = false;
static shared_memory_t shm;  // shared memory to write boolean value for whether
                             // or not a cat is detected

static bool run_thread = false;
static pthread_t cat_detector_thread;

/**
 * Run cat detection in a background thread.
 *
 * The camera will on turn on and look for a cat if
 * motion is been detected. Once the camera is on, it will
 * stay awake until no cat is detected for `timeout` seconds
 * set in CatDetector_init().
 *
 * Poll from CatDetector_catDetected() to see whether or not
 * a cat is detected.
 */
void* catDetectedThread(void* arg);

void CatDetector_init(uint16_t timeout_seconds) {
  assert(!is_initialized);

  shm = SharedMemory_init(SHARED_MEM_NAME);
  MotionSensor_init();
  CameraController_init();

  run_thread = true;
  int created =
      pthread_create(&cat_detector_thread, NULL, catDetectedThread, NULL);
  if (created == -1) {
    perror("Failed to create a motion sensor thread");
    exit(EXIT_FAILURE);
  }

  timeout = timeout_seconds;
  is_initialized = true;
}  // end of CatDetector_init()

void CatDetector_cleanup(void) {
  assert(is_initialized);

  run_thread = false;
  pthread_cancel(cat_detector_thread);
  int joined = pthread_join(cat_detector_thread, NULL);
  if (joined == -1) {
    perror("Failed to join motion sensor thread");
  }

  SharedMemory_cleanup(&shm);
  MotionSensor_cleanup();
  CameraController_cleanup();
  is_initialized = false;
}  // end of CatDetector_cleanup()

bool CatDetector_detected(void) {
  assert(is_initialized);
  return atomic_load(&cat_detected);
}  // end of CatDetector_detected()

void* catDetectedThread(void* arg) {
  (void)arg;

  while (run_thread) {
    if (MotionSensor_motionDetected()) {
      printf("Motion detected, starting camera...\n");
      CameraController_sendCommand(CAMERA_START);
      usleep(10000000);  // give some time for the camera to start up

      bool detected;
      time_t start_time = time(NULL);
      while (run_thread) {
        detected = SharedMemory_read(&shm) == 1;  // 1 for cat detected
        if (!detected && difftime(time(NULL), start_time) > timeout) {
          printf("No cat detected in the last %u seconds\n", timeout);
          break;
        } else if (detected) {
          atomic_store(&cat_detected, detected);
          start_time = time(NULL);
        }
      }

      // no cat detected within the timeout window
      CameraController_sendCommand(CAMERA_STOP);
      atomic_store(&cat_detected, false);
    }
  }
  return NULL;
}  // end of catDetectedThread()
