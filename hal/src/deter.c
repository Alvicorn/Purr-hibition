// Sample state machine for one GPIO pin.
//changed gpio.h to have Gpio_waitForSingleLineChange

#include "hal/deter.h"
#include "hal/gpio.h"
#include "../../app/include/beatgenerator.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>

// Pin config info: GPIO 5 (Rotary PUSH)
//   $ gpiofind GPIO5
//   >> gpiochip2 15
#define GPIO_CHIP          GPIO_CHIP_2
#define GPIO_LINE_NUMBER   17


static bool isInitialized = false;
static pthread_t id;
struct GpioLine* s_lineBtn = NULL;
static bool isDeterOn = false;

void deter_init(void)
{
    assert(!isInitialized);
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER);
    isInitialized = true;
    pthread_create(&id, NULL, &deter_doState, NULL);
}
void deter_cleanup(void)
{
    assert(isInitialized);
    isInitialized = false;
    pthread_cancel(id);
    pthread_join(id, NULL);
    Gpio_close(s_lineBtn);
}

void setDeter(bool value){
    isDeterOn = value;
    printf("isDeterOn: %d\n", isDeterOn);
}

static void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}
static long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}
void playDeter(){

    long long currentTime = getTimeInMs();
    BeatGenerator_setBeat(CUSTOM_BEAT);
    while(getTimeInMs() < currentTime + 10000) {
        Gpio_setValue(GPIO_CHIP,GPIO_LINE_NUMBER,1);
        sleepForMs(250);
        Gpio_setValue(GPIO_CHIP,GPIO_LINE_NUMBER,0);
        sleepForMs(250);
    }
    isDeterOn = false;
    BeatGenerator_setBeat(NO_BEAT);
}
// TODO: This should be on a background thread!
void* deter_doState()
{
    assert(isInitialized);

    while (true) {
        if (!isDeterOn){
            Gpio_setValue(GPIO_CHIP, GPIO_LINE_NUMBER,0);
            sleep(1);
        }
        else{
            printf("playing deter!\n");
            playDeter();
        }

    }

}