#include <iostream>
#include <signal.h>
#include "backtrace.hpp"

extern "C" void sig_handler(int)
{
	std::cout << "SIGGEGV catched\n";
	std::cout << utils::stacktrace{} << "\n";
	exit(1);
}

template <typename T> T h(T * p)
{
	if (p) {
		std::cout << utils::stacktrace{} << "\n";
		return T{};
	} else {
		return *p;
	}
}

namespace example
{
int f(int * p) { return h(p); }
}

int g(int * p) { return example::f(p); }

int main(int, char **)
{
	signal(SIGSEGV, sig_handler);

	std::cout << "Intentional output\n";
	int a = 99;
	int b = g(&a);
	std::cout << "a=" << a << ", b=" << b << "\n";

	int c = g(nullptr);
	std::cout << "c=" << c << "\n";

	return 0;
}
