#include <stdio.h>
#include "hal/udp.h"
#include <stdbool.h>

int main() {
  printf("running server!\n");
  Udp_init();
  while(true){
    // infinite loop to prevent terminating
  }
  return 0;
}