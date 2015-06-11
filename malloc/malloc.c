#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <time.h>

// #define USE_CLOCKTIME   1
#define USE_RDTSC       1

#define CLOCK_TYPE CLOCK_MONOTONIC

static __inline __attribute__((always_inline)) uint64_t rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo);
}

static double cyclesPerSec = 0;

static double getCyclesPerSec()
{
  struct timespec startTime, stopTime;
  uint64_t startCycles, stopCycles, nsecs;
  double cPSec;
  clock_gettime(CLOCK_TYPE, &startTime);
  startCycles = rdtsc();
  while (true) {
    clock_gettime(CLOCK_TYPE, &stopTime);
    stopCycles = rdtsc();
    nsecs = (stopTime.tv_nsec - startTime.tv_nsec) + 
            (stopTime.tv_sec - startTime.tv_sec) * 1000000000;
    if (nsecs > 1000000) {
      cPSec = 1000000000.0 * ((double) (stopCycles - startCycles)) / (double) nsecs;
      break;
    }
  }
  return cPSec;
}

static __inline __attribute__((always_inline)) double cyclesToSeconds(uint64_t cycles)
{
  if (cyclesPerSec == 0) {
    cyclesPerSec = getCyclesPerSec();
  }
  return (double) cycles / cyclesPerSec;
}

static __inline __attribute__((always_inline)) double cyclesToMicroSeconds(uint64_t cycles)
{
  if (cyclesPerSec == 0) {
    cyclesPerSec = getCyclesPerSec();
  }
  return 1000000.0 * (double) cycles / cyclesPerSec;
}

#define TIME(block, sec, ns) do {\
	struct timespec start;\
	struct timespec end;\
	clock_gettime(CLOCK_TYPE, &start);\
	(block);\
	clock_gettime(CLOCK_TYPE, &end);\
	(sec) = end.tv_sec - start.tv_sec;\
	(ns) = end.tv_nsec - start.tv_nsec;\
} while(0)


#define RDTSC(block, interval) do { \
  uint64_t start = rdtsc();         \
  (block);                          \
  (interval) = rdtsc() - start;     \
} while (0)

/* glibc malloc functions*/
extern void* __libc_malloc  (size_t size);
extern void  __libc_free    (void* ptr);
extern void* __libc_calloc  (size_t nmemb, size_t size);
extern void* __libc_realloc (void* ptr, size_t size);

/* my malloc hooks */
static void* malloc_hook  (size_t size);
static void  free_hook    (void* ptr);
static void* calloc_hook  (size_t nmemb, size_t size);
static void* realloc_hook (void* ptr, size_t size);

/* flags to disable hooks to avoid recursion */
static bool enable_malloc_hook = true;
static bool enable_free_hook  = true;
static bool enable_calloc_hook = true;
static bool enable_realloc_hook = true;

/* the hooks themselves */
static void* malloc_hook(size_t size) {
	void* ptr;
#ifdef USE_CLOCKTIME
	time_t sec;
	long ns;
	TIME(ptr = malloc(size), sec, ns);
	printf("Call to malloc(%zu) -> %p. Took %us, %luns\n", size, ptr, sec, ns);
#elif USE_RDTSC
  uint64_t interval;
  RDTSC(ptr = malloc(size), interval);
  double usecs = cyclesToMicroSeconds(interval);
	printf("Call to malloc(%zu) -> %p. Took %lu cycles, %lf.3 us\n", size, ptr, interval, usecs);
#else
  ptr = malloc(size);
#endif 
	return ptr;
}

static void free_hook(void* ptr) {
#ifdef USE_CLOCKTIME
	time_t sec;
	long ns;
	TIME(free(ptr), sec, ns);
	printf("Call to free(%p). Took %us, %luns\n", ptr, sec, ns);
#elif USE_RDTSC
  uint64_t interval;
  RDTSC(free(ptr), interval);
  double usecs = cyclesToMicroSeconds(interval);
	printf("Call to free(%p). Took %lu cycles, %lf.3 us\n", ptr, interval, usecs);
#else
  free(ptr);
#endif
}

static void* calloc_hook(size_t n, size_t size) {
	void* ptr;
#ifdef USE_CLOCKTIME
	time_t sec;
	long ns;
	TIME(ptr = calloc(n, size), sec, ns);
	printf("Call to calloc(%zu, %zu) -> %p. Took %us, %luns\n",
			n, size, ptr, sec, ns);
#elif USE_RDTSC
  uint64_t interval;
  RDTSC(ptr = calloc(n, size), interval);
  double usecs = cyclesToMicroSeconds(interval);
	printf("Call to calloc(%zu, %zu) -> %p. Took %lu cycles, %lf.3 us\n",
			n, size, ptr, interval, usecs);
#else
  ptr = calloc(n, size);
#endif
	return ptr;
}

static void* realloc_hook(void* old_ptr, size_t size) {
	void* ptr;
#ifdef USE_CLOCKTIME
	time_t sec;
	long ns;
	TIME(ptr = realloc(old_ptr, size), sec, ns);
	printf("Call to realloc(%p, %zu) -> %p. Took %us, %luns\n",
			old_ptr, size, ptr, sec, ns);
#elif USE_RDTSC
  uint64_t interval;
  RDTSC(ptr = realloc(old_ptr, size), interval);
  double usecs = cyclesToMicroSeconds(interval);
	printf("Call to realloc(%p, %zu) -> %p. Took %lu cycles, %lf.3 us\n",
			old_ptr, size, ptr, interval, usecs);
#else
  ptr = realloc(old_ptr, size);
#endif
	return ptr;
}

/* overriding functions to call hooks */
void* malloc(size_t size) {
	if(enable_malloc_hook == false) { return __libc_malloc(size); }

	enable_malloc_hook = false;
	void* ptr = malloc_hook(size);
	enable_malloc_hook = true;

	return ptr;
}

void free(void* ptr) {
	if(enable_free_hook == false) { __libc_free(ptr); return; } 

	enable_free_hook = false;
	free_hook(ptr);
	enable_free_hook = true;

	return;
}

void* calloc(size_t nmemb, size_t size) {
	if(enable_calloc_hook == false) { return __libc_calloc(nmemb, size); }

	enable_calloc_hook = false;
	void* ptr = calloc_hook(nmemb, size);
	enable_calloc_hook = true;

	return ptr;
}

void* realloc(void* ptr, size_t size) {
	if(enable_realloc_hook == false) { return __libc_realloc(ptr, size); }

	enable_realloc_hook = false;
	void* new_ptr = realloc_hook(ptr, size);
	enable_realloc_hook = true;

	return new_ptr;
}

