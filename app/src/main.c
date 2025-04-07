#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "byai_camera/catDetector.h"

void init_modules(void) { 
  CatDetector_init(10); 
}

void cleanup_modules(void) {
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
