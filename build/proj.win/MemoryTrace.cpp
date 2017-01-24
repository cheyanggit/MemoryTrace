// MemoryTrace.cpp : 定义控制台应用程序的入口点。
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



	A* a = new A();
	a->ma();

	dump_memory_info();
	report_memory_info(NULL);
	//a->~A();

	free(p);
	delete a;

    return 0;
}

