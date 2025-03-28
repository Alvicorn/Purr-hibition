#include "byai_camera/cameraController.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "hal/sharedMemory.h"

#define SHARED_MEM_COMMAND "/byai_cam_commands"
#define SHARED_MEM_STATE "/byai_cam_state"

/**
 * Valid states the BYAI camera is in.
 * See python/target/camera_coordinator.py for more details.
 */
enum BYAICameraState {
  NO_STATE = 0,
  KILLED = 1,
  SLEEPING = 2,
  RUNNING = 3,
  TOTAL_STATES = 4
};

static bool is_initialized = false;
static SharedMemory_t shm_command;
static SharedMemory_t shm_state;

void CameraController_init(void) {
  assert(!is_initialized);
  shm_command = SharedMemory_init(SHARED_MEM_COMMAND);
  shm_state = SharedMemory_init(SHARED_MEM_STATE);
  is_initialized = true;
}  // end of CameraController_init()

void CameraController_cleanup(void) {
  assert(is_initialized);
  SharedMemory_cleanup(&shm_command);
  SharedMemory_cleanup(&shm_state);
  is_initialized = false;
}  // end of CameraController_cleanup()

void CameraController_sendCommand(enum CameraCommand command) {
  assert(is_initialized);
  int current_cam_state = SharedMemory_read(&shm_state);

  if (current_cam_state <= NO_STATE || TOTAL_STATES <= current_cam_state) {
    printf("Reading an invalid BYAI camera state... %d\n", current_cam_state);
    return;
  }

  enum BYAICameraState cam_state = (enum BYAICameraState)current_cam_state;

  switch (cam_state) {
    case KILLED:
      printf("BYAI camera has been terminated. Can not restart the camera\n");
      return;
    case SLEEPING:
      if (command == CAMERA_START || command == CAMERA_KILL) {
        SharedMemory_write(&shm_command, command);
      } else {
        printf("BYAI camera is already alseep\n");
      }
      return;
    case RUNNING:
      if (command == CAMERA_KILL || command == CAMERA_STOP) {
        SharedMemory_write(&shm_command, command);
      } else {
        printf("BYAI camera is already running\n");
      }
      return;
    default:
      printf("Unknown BYAI camera control command: %d\n", command);
      return;
  }
}  // end of CameraController_sendCommand()
