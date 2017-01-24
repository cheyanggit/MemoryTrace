#ifndef  MEMORY_TRACER_X_H
#define  MEMORY_TRACER_X_H
#include "memory_define.h"

void * operator new(
	size_t size,
	int blockType,
	const char * fileName,
	int line
	);

void  operator delete(void* ptr, 
	int blockType,
	const char * fileName,
	int line);

void  operator delete(void* ptr);

void * operator new[](
	size_t size,
	int blockType,
	const char * fileName,
	int line
	);

void  operator delete[](void* ptr);

void  operator delete[](
	void* ptr,
	int blockType,
	const char * fileName,
	int line	
	);

#define MT_NEW new( MEMORY_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new MT_NEW

#endif //MEMORY_TRACER_X_H