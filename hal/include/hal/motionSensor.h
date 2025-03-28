#ifndef _MOTION_SENSOR_H_
#define _MOTION_SENSOR_H_

/**
 * Initalize the motion sensor.
 */
void MotionSensor_init(void);

/**
 * Free the resources required by the motion sensor.
 */
void MotionSensor_cleanup(void);

/**
 * Returns true is motion has been detected, false otherwise.
 * Recommend to poll this function ~0.5 seconds.
 */
bool MotionSensor_motionDetected(void);

#endif
