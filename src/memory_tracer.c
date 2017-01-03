#ifndef  MEMORY_TRACER_H
#define  MEMORY_TRACER_H
#include "memory_define.h"

#define  FILE_NAME_LENGTH			256
#define  OUTPUT_FILE				"leak_info.txt"


#if defined(WIN32)

#include <windows.h>
typedef CRITICAL_SECTION mutex_handle_t;
#define MUTEX_CREATE(handle) 	InitializeCriticalSection(&handle)
#define MUTEX_LOCK(handle) 		EnterCriticalSection(&handle)
#define MUTEX_UNLOCK(handle) 	LeaveCriticalSection(&handle)
#define MUTEX_DESTROY(handle)	DeleteCriticalSection(&handle)

#else 

#include <pthread.h>
typedef pthread_mutex_t mutex_handle_t;
#define MUTEX_CREATE(handle) pthread_mutex_init(&(handle), NULL)
#define MUTEX_LOCK(handle) pthread_mutex_lock(&(handle))
#define MUTEX_UNLOCK(handle) pthread_mutex_unlock(&(handle))
#define MUTEX_DESTROY(handle) pthread_mutex_destroy(&(handle))


#endif

static long sRequestCurr = 1;      /* Current request number */

static unsigned char sNoMansLandFill = 0xFD;   /* fill no-man's land with this */
static unsigned char sAlignLandFill  = 0xED;   /* fill no-man's land for aligned routines */
static unsigned char sDeadLandFill   = 0xDD;   /* fill free objects with this */
static unsigned char sCleanLandFill  = 0xCD;   /* fill new objects with this */

static MemoryBlockHeader * sFirstBlock;
static MemoryBlockHeader * sLastBlock;

static const char * const sBlockUseName[MEMORY_MAX_BLOCKS] = {
		"Free",
		"Normal",
		"CRT",
		"Ignore",
		"Client",
		};
static mutex_handle_t sMutexHandle;
static int sMutexCreated=0;
	
void * yh_malloc(unsigned int size, const char * file, unsigned int line)
{
	void * ptr=malloc(size);
	
}

void * yh_calloc(unsigned int elements, unsigned int size, const char * file, unsigned int line);
void * yh_realloc (void *ptr,int size,const char *file,unsigned int line);
char * yh_strdup (const char *ptr, const char * file, unsigned int line);
void yh_free(void * ptr);

#endif //MEMORY_TRACER_H