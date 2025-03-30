#include "hal/motionSensor.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#include "hal/gpio.h"

#define GPIO_CHIP GPIO_CHIP_1
#define GPIO_LINE_NUMBER_THIRTY_EIGHT 38

// note: gpiomon gpiochip1 38 for terminal monitoring
struct GpioLine* s_line_number_thirty_eight = NULL;

static bool motionSensorInitialized = false;
static bool continueCheckingMotion = true;

static atomic_bool motionDetected = ATOMIC_VAR_INIT(false);
static pthread_t thread;

/**
 * Continously check for motion in front of the sensor.
 */
void* detectMotionThread(void* arg);

void MotionSensor_init(void) {
  assert(!motionSensorInitialized);
  Gpio_initialize();
  s_line_number_thirty_eight =
      Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER_THIRTY_EIGHT);
  continueCheckingMotion = true;

  int created = pthread_create(&thread, NULL, detectMotionThread, NULL);
  if (created == -1) {
    perror("Failed to create a motion sensor thread");
    exit(EXIT_FAILURE);
  }

  motionSensorInitialized = true;
}  // end of MotionSensor_init()

void MotionSensor_cleanup(void) {
  assert(motionSensorInitialized);
  continueCheckingMotion = false;

  int joined = pthread_join(thread, NULL);
  if (joined == -1) {
    perror("Failed to join motion sensor thread");
  }
  Gpio_close(s_line_number_thirty_eight);
  Gpio_cleanup();
  motionSensorInitialized = false;
}  // end of MotionSensor_cleanup()

bool MotionSensor_motionDetected(void) {
  return atomic_load(&motionDetected);
}  // end of MotionSensor_motionDetected()

void* detectMotionThread(void* arg) {
  (void)arg;

  while (continueCheckingMotion) {
    struct gpiod_line_bulk bulkEvents;
    bool detected = false;
    int numEvents =
        Gpio_waitForLineChange(s_line_number_thirty_eight, &bulkEvents);

    for (int i = 0; i < numEvents; i++) {
      // Get the line handle for this event
      struct gpiod_line* line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

      // Get the line event
      struct gpiod_line_event event;
      if (gpiod_line_event_read(line_handle, &event) == -1) {
        perror("Line Event");
        exit(EXIT_FAILURE);
      }

      bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
      if (isRising) {
        detected = true;
        break;
      }
    }
    atomic_store(&motionDetected, detected);
  }
  return NULL;
}  // end of detectMotionThread()
