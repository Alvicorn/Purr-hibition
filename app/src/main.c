#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "byai_camera/catDetector.h"

#include "hal/udp.h"
#include "hal/deterrents.h"

void init_modules(void) { 
  CatDetector_init(10); 
  Deterrents_init();
  Udp_init();
}

void cleanup_modules(void) {
  Udp_cleanup();
  Deterrents_cleanup();
  CatDetector_cleanup(); 
}

int main() {
  printf("Starting Purr-hibition!\n");
  init_modules();

  int i = 0;
  while (true) {
    printf("%d: Cat detected: %d\n", i++, CatDetector_detected());
    usleep(500000);
  }

  cleanup_modules();
  return 0;
}