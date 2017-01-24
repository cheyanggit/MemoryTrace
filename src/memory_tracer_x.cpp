#include "memory_tracer.h"
#include <stdio.h>

void * operator new(
	size_t size,
	int blockType,
	const char * fileName,
	int line
	)
{
	return yh_malloc(size, blockType, fileName, line);
}

void  operator delete(void* ptr)
{
	yh_free(ptr, MEMORY_NORMAL_BLOCK);
}

void  operator delete(void* ptr,
	int blockType,
	const char * fileName,
	int line)
{
	yh_free(ptr, blockType);
}

void * operator new[](
	size_t size,
	int blockType,
	const char * fileName,
	int line
	)
{
	return yh_malloc(size, blockType, fileName, line);
}

void  operator delete[](void* ptr)
{
	yh_free(ptr, MEMORY_NORMAL_BLOCK);
}

void  operator delete[](
	void* ptr,
	int blockType,
	const char * fileName,
	int line
	)
{
	yh_free(ptr, blockType);
}