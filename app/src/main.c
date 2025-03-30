#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "byai_camera/catDetector.h"

void init_modules(void) { CatDetector_init(10); }

void cleanup_modules(void) { CatDetector_cleanup(); }

int main() {
  printf("Starting Purr-hibition!\n");
  init_modules();

  while (true) {
    printf("Cat detected: %d\n", CatDetector_detected());
    usleep(500000);
  }

  cleanup_modules();
  return 0;
}
