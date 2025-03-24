#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "hal/cat_detector.h"
#include "hal/gpio.h"
#include "hal/motionSensor.h"

/**
 * Function to run on a thread to continuously poll
 * if a cat is detected or not.
 */
void* detect_a_cat(void* arg) {
  (void)arg;

  while (true) {
    printf("Cat detected: %d\n", CatDetector_detected());
    usleep(500000);
  }
}

int main() {
  printf("Starting Purr-hibition!\n");

  // setup modules
  Gpio_initialize();
  MotionSensor_init();
  CatDetector_init();

  // main
  pthread_t cat_detection_thread;
  pthread_t motion_sensor_thread;

  pthread_create(&cat_detection_thread, NULL, detect_a_cat, NULL);

  while (true) {
  }

  pthread_cancel(cat_detection_thread);
  pthread_join(cat_detection_thread, NULL);

  MotionSensor_doState();

  // clean up resources
  CatDetector_cleanup();
  MotionSensor_cleanup();
  Gpio_cleanup();

  return 0;
}
