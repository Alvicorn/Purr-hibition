#ifndef __SHARED_MEMORY_H_
#define __SHARED_MEMORY_H_

/**
 * SharedMemory_t struct maintains the values
 * required to read and write to a shared memory space.
 */
typedef struct {
  int fd;
  int* shared_mem;
  char* shared_mem_name;

} SharedMemory_t;

/**
 * Create a SharedMemory_t instance  and initalize the
 * internal fields.
 *
 * @param shared_mem_name name of the shared memory space.
 */
SharedMemory_t SharedMemory_init(char* shared_mem_name);

/**
 * Clean up the resources allocated to a
 * SharedMemory_t instance.
 *
 * @param shm SharedMemory_t instance to be cleaned up.
 */
void SharedMemory_cleanup(SharedMemory_t* shm);

/**
 * Read the value in the shared memory space.
 *
 * @param shm SharedMemory_t instance.
 */
int SharedMemory_read(SharedMemory_t* shm);

/**
 * Write a value to the shared memory space.
 *
 * @param shm SharedMemory_t instance.
 * @param value integer to be written to the shared memory space.
 */
void SharedMemory_write(SharedMemory_t* shm, int value);

#endif
