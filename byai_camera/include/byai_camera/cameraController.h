#ifndef __CAMERA_CONTROLLER_H_
#define __CAMERA_CONTROLLER_H_

/**
 * Available commands to control the camera.
 * See python/target/camera_coordinator.py for more details.
 */
enum CameraCommand { CAMERA_KILL = 1, CAMERA_START = 2, CAMERA_STOP = 3 };

/**
 * Initalize the camera controller.
 */
void CameraController_init(void);

/**
 * Clean up the resources used by the CameraController.
 */
void CameraController_cleanup(void);

/**
 * Send a command to the BYAI camera.
 *
 * @param command Instruction to be sent to control the camera.
 */
void CameraController_sendCommand(enum CameraCommand command);

#endif
