#ifndef _DETERRENTS_H_
#define _DETERRENTS_H_

#include <stdbool.h>

/**
* Initialize deterrents.
*
* Avaliable Deterrents:
*   - Flashing LED light
*   - Repulsive audio
*/
void Deterrents_init(void);



void Deterrents_cleanup(void);

/**
* Activate Deterrents. Deterrents will happen if a cat is detected.
*/
void Deterrents_activate_deterrents(void);


/**
* Stop all deterrents, Deterrents will not happen if a cat is detected.
*/
void Deterrents_cancel_deterrents(void);


/**
 * For use in udp.c, to send info to web app about deterrent status.
*/

bool Deterrents_check_deterrents_status(void);

#endif