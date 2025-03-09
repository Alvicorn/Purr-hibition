#include <stdio.h>
#include "hal/motionSensor.h"
#include "hal/gpio.h"

int main() {
  printf("Hello world!\n");

    Gpio_initialize();
    MotionSensor_init();
    MotionSensor_doState();
    MotionSensor_cleanup();
    Gpio_cleanup();

  return 0;
}
