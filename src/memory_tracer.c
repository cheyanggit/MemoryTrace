#ifndef  MEMORY_TRACER_H
#define  MEMORY_TRACER_H
#include "memory_define.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define  FILE_NAME_LENGTH			256
#define  OUTPUT_FILE				"leak_info.txt"
#define MEMORY_ALLOCATION_FILE_LINENUM "\nMemory allocated at %hs(%d).\n"


#if defined(_WIN32)

#include <windows.h>
typedef CRITICAL_SECTION mutex_handle_t;
#define MUTEX_CREATE(handle) 	InitializeCriticalSection(&handle)
#define MUTEX_LOCK(handle) 		EnterCriticalSection(&handle)
#define MUTEX_UNLOCK(handle) 	LeaveCriticalSection(&handle)
#define MUTEX_DESTROY(handle)	DeleteCriticalSection(&handle)
#define MUTEX_INITILIZER(m) \
		static unsigned int volatile m_init = 0; \
		static bool volatile m_wait = 1; \
		if (!InterlockedExchange(&m_init, 1)) \
		{ mutex_init(&m); m_wait = 0; } \
		while(m_wait);  

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
static unsigned char sAlignLandFill = 0xED;   /* fill no-man's land for aligned routines */
static unsigned char sDeadLandFill = 0xDD;   /* fill free objects with this */
static unsigned char sCleanLandFill = 0xCD;   /* fill new objects with this */

static MemoryBlockHeader * sFirstBlock;
static MemoryBlockHeader * sLastBlock;

static const char * const sBlockUseName[MEMORY_MAX_BLOCKS] = {
		"Free",
		"Normal",
		"CRT",
		"Ignore",
		"Client",
};
#if defined(WIN32)
static mutex_handle_t sMutexHandle;
#else
static mutex_handle_t sMutexHandle = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined(WIN32)
static void init_mutex_handle()
{
	static int mutex_created = 0;
	static int mutex_wait = 1;
	if (!InterlockedExchange(&mutex_created, 1))
	{
		MUTEX_CREATE(sMutexHandle);
		mutex_wait = 0;
	}
	while (mutex_wait)
	{
		Sleep(0);
	}
}
#endif

static int  check_bytes(unsigned char * pb, unsigned char bCheck, size_t nSize)
{
	while (nSize--)
	{
		if (*pb++ != bCheck)
		{
			return 0;
		}
	}
	return 1;
}

void add_memory_info(MemoryBlockHeader * head, unsigned int size, int blockType, const char * file, unsigned int line)
{
	if (head)
	{
#if defined(_WIN32)
		init_mutex_handle();
#endif

		head->fileName = (char*)file;
		head->line = line;
		head->dataSize = size;
		head->blockUse = blockType;

		MUTEX_LOCK(sMutexHandle);

		head->request = sRequestCurr++;

		if (sFirstBlock)
			sFirstBlock->blockHeaderPrev = head;
		else
			sLastBlock = head;

		head->blockHeaderNext = sFirstBlock;
		head->blockHeaderPrev = NULL;


		sFirstBlock = head;

		memset((void*)head->gap, sNoMansLandFill, Memory_NoMansLandSize);
		memset(Memory_GetDataPtr(head) + size, sNoMansLandFill, Memory_NoMansLandSize);
		memset((void*)Memory_GetDataPtr(head), sCleanLandFill, size);

		MUTEX_UNLOCK(sMutexHandle);
	}
}

void modify_memory_info(MemoryBlockHeader * newBlock, MemoryBlockHeader * oldBlock, unsigned int newSize, int blockType, const char * file, unsigned int line)
{

	unsigned char * blockData;
	if (newBlock && oldBlock)
	{
#if defined(_WIN32)
		init_mutex_handle();
#endif

		newBlock->fileName = (char*)file;
		newBlock->line = line;


		MUTEX_LOCK(sMutexHandle);

		newBlock->request = sRequestCurr++;

		blockData = Memory_GetDataPtr(newBlock);

		if (newSize > newBlock->dataSize)
		{
			memset(blockData + newBlock->dataSize, sCleanLandFill, newSize - newBlock->dataSize);
		}

		memset(blockData + newSize, sNoMansLandFill, Memory_NoMansLandSize);

		newBlock->dataSize = newSize;

		if (newBlock == oldBlock)
		{
			return;
		}

		if (newBlock->blockHeaderNext)
		{
			newBlock->blockHeaderNext->blockHeaderPrev = newBlock->blockHeaderPrev;
		}
		else
		{
			//assert(sLastBlock == oldBlock);
			sLastBlock = newBlock->blockHeaderPrev;
		}

		if (newBlock->blockHeaderPrev)
		{
			newBlock->blockHeaderPrev->blockHeaderNext = newBlock->blockHeaderNext;
		}
		else
		{
			//assert(sFirstBlock == oldBlock);
			sFirstBlock = newBlock->blockHeaderNext;
		}

		newBlock->blockHeaderNext = sFirstBlock;
		newBlock->blockHeaderPrev = NULL;

		MUTEX_UNLOCK(sMutexHandle);
	}
}

void yh_free(void * ptr, int blockType);

void * yh_malloc(unsigned int size, int blockType, const char * file, unsigned int line)
{

	void* retval = NULL;
	size_t blockSize = sizeof(MemoryBlockHeader) + size + Memory_NoMansLandSize;
	MemoryBlockHeader * head = (MemoryBlockHeader *)malloc(blockSize);
	if (head)
	{
		add_memory_info(head, size, blockType, file, line);

		//memset((void*)head->gap, sNoMansLandFill, Memory_NoMansLandSize);
		//memset((void*)Memory_GetDataPtr(head) + size, sNoMansLandFill, Memory_NoMansLandSize);
		//memset((void*)Memory_GetDataPtr(head), sCleanLandFill, size);

		retval = (void*)Memory_GetDataPtr(head);
	}

	return retval;
}

void * yh_calloc(unsigned int elements, unsigned int size, int blockType, const char * file, unsigned int line)
{
	void* block;
	size *= elements;
	block = yh_malloc(size, blockType, file, line);
	if (block != NULL)
	{
		memset(block, 0, size);
	}
	return block;
}

void * yh_realloc(void *ptr, int newSize, int blockType, const char *file, unsigned int line)
{

	MemoryBlockHeader* oldBlock;
	MemoryBlockHeader* newBlock;

	if (ptr == NULL)
	{
		return yh_malloc(newSize, blockType, file, line);
	}

	if (newSize == 0)
	{
		yh_free(ptr, blockType);
		return NULL;
	}

	oldBlock = Memory_GetBlockPtr(ptr);
	newBlock = (MemoryBlockHeader*)realloc(oldBlock, sizeof(MemoryBlockHeader) + newSize + Memory_NoMansLandSize);
	if (newBlock != NULL)
	{
		modify_memory_info(newBlock, oldBlock, blockType, newSize, file, line);
		return Memory_GetDataPtr(newBlock);
	}

	return NULL;

}
char * yh_strdup(const char *str, const char * file, unsigned int line)
{
	char* ptr = (char*)yh_malloc(strlen(str) + 1, MEMORY_NORMAL_BLOCK, file, line);
	if (ptr != NULL)
	{
		//strcpy(ptr, str);
		memmove(ptr, str, strlen(str) + 1);
	}

	return ptr;
}

void yh_free(void * ptr, int blockType)
{
	MemoryBlockHeader* head;

	if (ptr == NULL)
	{
		return;
	}

	head = Memory_GetBlockPtr(ptr);

	if (!check_bytes(head->gap, sNoMansLandFill, Memory_NoMansLandSize))
	{
		if (head->fileName)
		{
			printf("HEAP CORRUPTION DETECTED: before %hs block (#%d) at 0x%p.\n"
				"CRT detected that the application wrote to memory before start of heap buffer.\n"
				MEMORY_ALLOCATION_FILE_LINENUM,
				sBlockUseName[MEMORY_BLOCK_TYPE(head->blockUse)],
				head->request,
				Memory_GetDataPtr(head),
				head->fileName,
				head->line);
		}
		else
		{
			printf("HEAP CORRUPTION DETECTED: before %hs block (#%d) at 0x%p.\n"
				"CRT detected that the application wrote to memory before start of heap buffer.\n",
				sBlockUseName[MEMORY_BLOCK_TYPE(head->blockUse)],
				head->request,
				Memory_GetDataPtr(head));
		}
	}

	if (!check_bytes(Memory_GetDataPtr(head) + head->dataSize, sNoMansLandFill, Memory_NoMansLandSize))
	{
		if (head->fileName)
		{
			printf("HEAP CORRUPTION DETECTED: after %hs block (#%d) at 0x%p.\n"
				"CRT detected that the application wrote to memory after end of heap buffer.\n"
				MEMORY_ALLOCATION_FILE_LINENUM,
				sBlockUseName[MEMORY_BLOCK_TYPE(head->blockUse)],
				head->request,
				Memory_GetDataPtr(head),
				head->fileName,
				head->line);
		}
		else
		{
			printf("HEAP CORRUPTION DETECTED: after %hs block (#%d) at 0x%p.\n"
				"CRT detected that the application wrote to memory after end of heap buffer.\n",
				sBlockUseName[MEMORY_BLOCK_TYPE(head->blockUse)],
				head->request,
				Memory_GetDataPtr(head));
		}
	}

#if defined(_WIN32)
	init_mutex_handle();
#endif

	MUTEX_LOCK(sMutexHandle);
	if (head->blockUse == MEMORY_IGNORE_BLOCK)
	{
		memset(head, sDeadLandFill, sizeof(MemoryBlockHeader) + head->dataSize + Memory_NoMansLandSize);
		free(head);
		return;
	}

	if (head->blockHeaderNext)
	{
		head->blockHeaderNext->blockHeaderPrev = head->blockHeaderPrev;
	}
	else
	{
		assert(sLastBlock == head);
		sLastBlock = head->blockHeaderPrev;
	}

	if (head->blockHeaderPrev)
	{
		head->blockHeaderPrev->blockHeaderNext = head->blockHeaderNext;
	}
	else
	{
		assert(sFirstBlock == head);
		sFirstBlock = head->blockHeaderNext;
	}

	memset(head, sDeadLandFill, sizeof(MemoryBlockHeader) + head->dataSize + Memory_NoMansLandSize);
	free(head);
	MUTEX_UNLOCK(sMutexHandle);
}

int check_memory()
{
	int allOkay;
	MemoryBlockHeader* head;
	unsigned char* blockUse;
	int okay;

#if defined(_WIN32)
	init_mutex_handle();
#endif

	MUTEX_LOCK(sMutexHandle);
	allOkay = 1;
	for (head = sFirstBlock; head != NULL; head = head->blockHeaderNext)
	{
		okay = 1;
		
		if (MEMORY_BLOCK_TYPE_IS_VALID(head->blockUse))
		{
			blockUse = (unsigned char*)sBlockUseName[MEMORY_BLOCK_TYPE(head->blockUse)];
		}
		else
		{
			blockUse = "DAMAGED";
		}

		if (!check_bytes(head->gap, sNoMansLandFill, Memory_NoMansLandSize))
		{
			if (head->fileName)
			{
				printf("HEAP CORRUPTION DETECTED: before %hs block (#%d) at 0x%p.\n"
					"CRT detected that the application wrote to memory before start of heap buffer.\n"
					MEMORY_ALLOCATION_FILE_LINENUM,
					blockUse,
					head->request,
					Memory_GetDataPtr(head),
					head->fileName,
					head->line);
			}
			else
			{
				printf("HEAP CORRUPTION DETECTED: before %hs block (#%d) at 0x%p.\n"
					"CRT detected that the application wrote to memory before start of heap buffer.\n",
					blockUse,
					head->request,
					Memory_GetDataPtr(head));
			}
			okay = 0;
		}

		if (!check_bytes(Memory_GetDataPtr(head) + head->dataSize, sNoMansLandFill, Memory_NoMansLandSize))
		{
			if (head->fileName)
			{
				printf("HEAP CORRUPTION DETECTED: after %hs block (#%d) at 0x%p.\n"
					"CRT detected that the application wrote to memory after end of heap buffer.\n"
					MEMORY_ALLOCATION_FILE_LINENUM,
					blockUse,
					head->request,
					Memory_GetDataPtr(head),
					head->fileName,
					head->line);
			}
			else
			{
				printf("HEAP CORRUPTION DETECTED: after %hs block (#%d) at 0x%p.\n"
					"CRT detected that the application wrote to memory after end of heap buffer.\n",
					blockUse,
					head->request,
					Memory_GetDataPtr(head));
			}
			okay = 0;
		}

		/* free blocks should remain undisturbed */
		if (head->blockUse == MEMORY_FREE_BLOCK &&
			!check_bytes(Memory_GetDataPtr(head), sDeadLandFill, head->dataSize))
		{
			if (head->fileName)
			{
				printf("HEAP CORRUPTION DETECTED: on top of Free block at 0x%p.\n"
					"CRT detected that the application wrote to a heap buffer that was freed.\n"
					MEMORY_ALLOCATION_FILE_LINENUM,
					Memory_GetDataPtr(head),
					head->fileName,
					head->line);
			}
			else
			{
				printf("HEAP CORRUPTION DETECTED: on top of Free block at 0x%p.\n"
					"CRT detected that the application wrote to a heap buffer that was freed.\n",
					Memory_GetDataPtr(head));
			}
			okay = 0;
		}

		if (!okay)
		{
			if (head->fileName)
			{
				printf("%hs located at 0x%p is %Iu bytes long.\n"
					MEMORY_ALLOCATION_FILE_LINENUM,
					blockUse,
					Memory_GetDataPtr(head),
					head->dataSize,
					head->fileName,
					head->line);
			}
			else
			{
				printf("%hs located at 0x%p is %Iu bytes long.\n",
					blockUse,
					Memory_GetDataPtr(head),
					head->dataSize);
			}

			allOkay = 0;
		}
	}

	MUTEX_UNLOCK(sMutexHandle);

	return allOkay;
}

void dump_memory_info()
{
	MemoryBlockHeader* head;
//#if defined(_WIN32)
//	init_mutex_handle();
//#endif
	//MUTEX_LOCK(sMutexHandle);

	printf("%s\n", "Memory Trace Info");
	printf("%s\n", "-----------------------------------------------------------");

	for (head = sFirstBlock; head != NULL; head = head->blockHeaderNext)
	{
		if (head->fileName != NULL)
		{
			printf("%s(%d) : ", head->fileName, head->line);
		}

		printf("block at 0x%p,subtype %x,%Iu bytes long.\n", Memory_GetDataPtr(head), head->blockUse, head->dataSize);
	}

	//MUTEX_UNLOCK(sMutexHandle);
}

void report_memory_info(const char* out_file)
{
	MemoryBlockHeader* head;
	char info[2048];

	FILE* fp_write = fopen(out_file?out_file:OUTPUT_FILE,"wt");
	
#if defined(_WIN32)
	init_mutex_handle();
#endif
	
	MUTEX_LOCK(sMutexHandle);

	sprintf(info, "%s\n", "Memory Trace Info");
	fwrite(info, strlen(info), 1, fp_write);

	sprintf(info, "%s\n", "-----------------------------------------------------------");
	fwrite(info, strlen(info), 1, fp_write);

	for (head = sFirstBlock; head != NULL; head = head->blockHeaderNext)
	{
		if (head->fileName != NULL)
		{
			sprintf(info, "%s(%d) : ",head->fileName,head->line);
			fwrite(info, strlen(info), 1, fp_write);
		}
		
		sprintf(info, "block at 0x%p,subtype %x,%Iu bytes long.\n", Memory_GetDataPtr(head),head->blockUse,head->dataSize);
		fwrite(info, strlen(info), 1, fp_write);
	}

	fflush(fp_write);
	fclose(fp_write);

	MUTEX_UNLOCK(sMutexHandle);
}

#endif //MEMORY_TRACER_H