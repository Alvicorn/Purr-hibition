#include <stdbool.h>
#include "audiomixer.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "beatgenerator.h"

#define MILLISECONDS_TO_NANOSECONDS 1000000
#define SECONDS_TO_MILLISECONDS 1000

static bool beatGeneratorInitialized = false;
static bool continueGeneratingBeat = true;
static pthread_t beatGeneratorThreadId;
static struct timespec ts;

typedef struct {
    wavedata_t soundWaveData;
    char soundPath[100];
} sound_t;

typedef struct {
    beatName_t beatName;
    sound_t sound[3];
} beat_t;

static beatName_t currentBeat = NO_BEAT;
static _Atomic int currentBPM = 120;
static pthread_mutex_t bpmMutex = PTHREAD_MUTEX_INITIALIZER;

static beat_t beats[] = {
    {
        .beatName = ROCK_BEAT,
        .sound = {
            {.soundPath = "beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav"},
            {.soundPath = "beatbox-wave-files/100053__menegass__gui-drum-cc.wav"},
            {.soundPath = "beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav"}
        }
    },
    {
        .beatName = CUSTOM_BEAT,
        .sound = {
            {.soundPath = "beatbox-wave-files/100052__menegass__gui-drum-bd-soft.wav"},
            {.soundPath = "beatbox-wave-files/100058__menegass__gui-drum-snare-hard.wav"},
            {.soundPath = "beatbox-wave-files/100066__menegass__gui-drum-tom-mid-hard.wav"}
        }
    }
};


void* beatGeneratorThread(void* _arg)
{
    (void)_arg;

    while(continueGeneratingBeat){

        if(currentBeat != NO_BEAT){

            AudioMixer_queueSound(&beats[currentBeat].sound[0].soundWaveData);
            AudioMixer_queueSound(&beats[currentBeat].sound[1].soundWaveData);
            AudioMixer_queueSound(&beats[currentBeat].sound[2].soundWaveData);
            nanosleep(&ts, NULL);
        }
    }
    return NULL;
}

int BeatGenerator_getBPM(void)
{
    pthread_mutex_lock(&bpmMutex);
    int currentBPMCopy = currentBPM;
    pthread_mutex_unlock(&bpmMutex);

    return currentBPMCopy;
}

bool BeatGenerator_setBPM(int bpm)
{
    if(bpm < 40 || bpm > 300){
        return false;
    }

    float currentSleepInNanoseconds = 60.0 / bpm / 2 * SECONDS_TO_MILLISECONDS * MILLISECONDS_TO_NANOSECONDS;  

    // printf("currentSleepInNanoseconds: %lf\n", currentSleepInNanoseconds);

    ts.tv_sec = 0;
    ts.tv_nsec = currentSleepInNanoseconds;

    currentBPM = bpm;
    return true;
}

bool BeatGenerator_incrementBPM(void)
{
    pthread_mutex_lock(&bpmMutex);
    int newBPM = currentBPM + 5;
    bool isNewBPMSet = BeatGenerator_setBPM(newBPM);
    pthread_mutex_unlock(&bpmMutex);
    return isNewBPMSet;
}

bool BeatGenerator_decrementBPM(void)
{
    pthread_mutex_lock(&bpmMutex);
    int newBPM = currentBPM - 5;
    bool isNewBPMSet = BeatGenerator_setBPM(newBPM);
    pthread_mutex_unlock(&bpmMutex);
    return isNewBPMSet;
}


void BeatGenerator_setBeat(beatName_t beat)
{
    currentBeat = beat;
}

const char* BeatGenerator_getBeat(void){
    switch (currentBeat){
        case ROCK_BEAT:
            return "Rock Beat";
        case CUSTOM_BEAT:
            return "Custom Beat";
        case NO_BEAT:
            return "None";
    }
    return "Missing beat";
}

void BeatGenerator_switchBeat(){
    switch (currentBeat){
        case ROCK_BEAT:
            currentBeat = CUSTOM_BEAT;
            break;
        case CUSTOM_BEAT:
            currentBeat = NO_BEAT;
            break;
        case NO_BEAT:
            currentBeat = ROCK_BEAT;
            break;
    }
}
int BeatGenerator_getBeatAsInt(void){
    if (currentBeat == NO_BEAT)
        return 0;
    else if (currentBeat == ROCK_BEAT)
        return 1;
    else
        return 2;
}


void BeatGenerator_init(void)
{
    if(beatGeneratorInitialized){
        return;
    }

    continueGeneratingBeat = true;

    for(int i = 0; i < 2; i++){
        AudioMixer_readWaveFileIntoMemory(beats[i].sound[0].soundPath, &beats[i].sound[0].soundWaveData);
        AudioMixer_readWaveFileIntoMemory(beats[i].sound[1].soundPath, &beats[i].sound[1].soundWaveData);
        AudioMixer_readWaveFileIntoMemory(beats[i].sound[2].soundPath, &beats[i].sound[2].soundWaveData);
    }

    BeatGenerator_setBPM(currentBPM);
    pthread_create(&beatGeneratorThreadId, NULL, beatGeneratorThread, NULL);
    beatGeneratorInitialized = true;
}

void BeatGenerator_cleanup(void)
{
    continueGeneratingBeat = false;
    pthread_join(beatGeneratorThreadId, NULL);

    for(int i = 0; i < 2; i++){
        AudioMixer_freeWaveFileData(&beats[i].sound[0].soundWaveData);
        AudioMixer_freeWaveFileData(&beats[i].sound[1].soundWaveData);
        AudioMixer_freeWaveFileData(&beats[i].sound[2].soundWaveData);
    }
    beatGeneratorInitialized = false;
}

void queueHiHatSound(void)
{
    AudioMixer_queueSound(&beats[ROCK_BEAT].sound[1].soundWaveData);
}

void queueSnareSound(void)
{
    AudioMixer_queueSound(&beats[ROCK_BEAT].sound[2].soundWaveData);
}

void queueBaseSound(void)
{
    AudioMixer_queueSound(&beats[ROCK_BEAT].sound[0].soundWaveData);
}