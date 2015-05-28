#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <time.h>

#define CLOCK_TYPE CLOCK_MONOTONIC

#define TIME(block, sec, ns) do {\
	struct timespec start;\
	struct timespec end;\
	clock_gettime(CLOCK_TYPE, &start);\
	(block);\
	clock_gettime(CLOCK_TYPE, &end);\
	(sec) = end.tv_sec - start.tv_sec;\
	(ns) = end.tv_nsec - start.tv_nsec;\
} while(0)


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
	time_t sec;
	long ns;

	void* ptr;

	TIME(ptr = malloc(size), sec, ns);

	printf("Call to malloc(%u) -> %p. Took %us, %luns\n", size, ptr, sec, ns);

	return ptr;
}

static void free_hook(void* ptr) {
	time_t sec;
	long ns;

	TIME(free(ptr), sec, ns);

	printf("Call to free(%p). Took %us, %luns\n", ptr, sec, ns);
}

static void* calloc_hook(size_t n, size_t size) {
	time_t sec;
	long ns;

	void* ptr;

	TIME(ptr = calloc(n, size), sec, ns);

	printf("Call to calloc(%u, %u) -> %p. Took %us, %luns\n",
			n, size, ptr, sec, ns);
	return ptr;
}

static void* realloc_hook(void* old_ptr, size_t size) {
	time_t sec;
	long ns;

	void* ptr;

	TIME(ptr = realloc(old_ptr, size), sec, ns);
	
	printf("Call to realloc(%p, %u) -> %p. Took %us, %luns\n",
			old_ptr, size, ptr, sec, ns);
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

