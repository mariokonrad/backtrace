#include <iostream>
#include "backtrace.hpp"

template <typename T> T h(T * p)
{
	std::cout << utils::stacktrace{} << "\n";
	return *p;
}

namespace example
{
int f(int * p) { return h(p); }
}

int g(int * p) { return example::f(p); }

int main(int, char **)
{
	int a = 99;
	int b = g(&a);

	std::cout << "a=" << a << ", b=" << b << "\n";
	return 0;
}
