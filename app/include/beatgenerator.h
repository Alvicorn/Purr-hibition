#ifndef BEAT_MIXER_H
#define BEAT_MIXER_H

#include <stdbool.h>

typedef enum {
    ROCK_BEAT,
    CUSTOM_BEAT,
    NO_BEAT,
} beatName_t;

void BeatGenerator_init(void);
void BeatGenerator_cleanup(void);

int BeatGenerator_getBPM(void);
// beat will be defaulted to 120BPM, but when set, can be in the range [40, 300] BPM
bool BeatGenerator_setBPM(int beat);
bool BeatGenerator_incrementBPM(void);
bool BeatGenerator_decrementBPM(void);
void BeatGenerator_setBeat(beatName_t beat);
const char* BeatGenerator_getBeat(void);
void BeatGenerator_switchBeat(void);
int BeatGenerator_getBeatAsInt(void);
void queueHiHatSound(void);
void queueSnareSound(void);
void queueBaseSound(void);

#endif