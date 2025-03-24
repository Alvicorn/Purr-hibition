#include <assert.h>
#include <stdio.h>
#include "hal/motionSensor.h"
#include "hal/gpio.h"

#define GPIO_CHIP GPIO_CHIP_1
#define GPIO_LINE_NUMBER_THIRTY_EIGHT 38

// note: gpiomon gpiochip1 38 for terminal monitoring

struct GpioLine* s_line_number_thirty_eight = NULL;

static bool motionSensorInitialized = false;
static bool continueCheckingMotion = true;

void MotionSensor_init(){
    assert(!motionSensorInitialized);
    s_line_number_thirty_eight = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER_THIRTY_EIGHT);
    continueCheckingMotion = true;
    // create a thread here
    motionSensorInitialized = true;
}


void MotionSensor_cleanup(){
    assert(motionSensorInitialized);
    continueCheckingMotion = false;
    motionSensorInitialized = false;
    // join the thread here
    Gpio_close(s_line_number_thirty_eight);
    Gpio_cleanup();
}


void MotionSensor_doState(){
    assert(motionSensorInitialized);

    while(continueCheckingMotion){
        struct gpiod_line_bulk bulkEvents;

        int numEvents = Gpio_waitForLineChange(s_line_number_thirty_eight, &bulkEvents);

        for(int i = 0; i < numEvents; i++){

            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // // Get the number of this line
            // unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle,&event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
            if(isRising){
                // call the function which checks if camera is active and activates it if inactive
                printf("Motion Detected! Turn on the camera!\n");
            }
            // for testing purposes only
            // if(!isRising){
            //     printf("Falling Edge!\n");
            // }
        }
    }
}