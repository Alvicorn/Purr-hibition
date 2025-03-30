#ifndef __SHARED_MEMORY_H_
#define __SHARED_MEMORY_H_

/**
 * shared_memory_t struct maintains the values
 * required to read and write to a shared memory space.
 */
typedef struct {
  int fd;
  int* shared_mem;
  char* shared_mem_name;

} shared_memory_t;

/**
 * Create a shared_memory_t instance  and initalize the
 * internal fields.
 *
 * @param shared_mem_name name of the shared memory space.
 */
shared_memory_t SharedMemory_init(char* shared_mem_name);

/**
 * Clean up the resources allocated to a
 * shared_memory_t instance.
 *
 * @param shm shared_memory_t instance to be cleaned up.
 */
void SharedMemory_cleanup(shared_memory_t* shm);

/**
 * Read the value in the shared memory space.
 *
 * @param shm shared_memory_t instance.
 */
int SharedMemory_read(shared_memory_t* shm);

/**
 * Write a value to the shared memory space.
 *
 * @param shm shared_memory_t instance.
 * @param value integer to be written to the shared memory space.
 */
void SharedMemory_write(shared_memory_t* shm, int value);

#endif
