// MemoryTrace.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "memory_tracer.h"

int main()
{
	char* p = (char*)malloc(6);
	memmove(p, "test", 5);
    return 0;
}

