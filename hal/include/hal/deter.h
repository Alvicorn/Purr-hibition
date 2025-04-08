#ifndef _deter_H_
#define _deter_H_

#include <stdbool.h>
#include <hal/gpio.h>
void deter_init(void);
void deter_cleanup(void);


// TODO: This should be on a background thread (internal?)
void* deter_doState();

void setDeter(bool);

#endif