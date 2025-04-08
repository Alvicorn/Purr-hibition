#include "hal/deterrents.h"
#include "hal/deter.h"
#include "byai_camera/catDetector.h"

#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>


static bool isInitialized = false;
static bool is_running = true;
static pthread_t thread;
static bool useDeterrents = true;
// static bool deterrentsIsRunningCurrently = false;


void* handleDeterrentsThread(void* arg) {
    (void)arg;
    printf("Cat Detected! Activating deterrents...\n");
    while(is_running){
        if(useDeterrents && CatDetector_detected()){
            printf("Cat Detected! Activating deterrents...\n");
            // call a function to activate the deterrents if they are not on
            setDeter(true);
        }
        else{
            setDeter(false);
        }
    }
    return NULL;
}


void Deterrents_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    pthread_create(&thread, NULL, handleDeterrentsThread, NULL);
}


void Deterrents_cleanup(void) {
    assert(isInitialized);
  
    is_running = false;
    pthread_cancel(thread);
    int joined = pthread_join(thread, NULL);
    if (joined == -1) {
        perror("Failed to join detterents thread");
    }
}


void Deterrents_activate_deterrents(void) {
    assert(isInitialized);
    useDeterrents = true;
}


void Deterrents_cancel_deterrents() {
    assert(isInitialized);
    useDeterrents = false;
}

bool Deterrents_check_deterrents_status(){
    assert(isInitialized);
    return useDeterrents;
}