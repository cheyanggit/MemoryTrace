// MemoryTrace.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "memory_tracer.h"
#include "memory_tracer_x.h"

class A
{
public:
	A()
	{

	}

	void ma()
	{
		printf("ma");
	}
};

int main()
{
	char* p = (char*)malloc(6);
	memmove(p, "test", 5);

	free(p);

	A* a = new A();
	a->ma();

	//a->~A();
	delete a;

    return 0;
}

