#include <iostream>
#include <vector>
#include <execinfo.h>
#include <cxxabi.h>

namespace utils
{
struct stacktrace {
	friend std::ostream & operator<<(std::ostream & os, const stacktrace & s);

	std::ostream & operator()(std::ostream & os, const int max_trace = 31,
		const size_t max_func_name_size = 256) const
	{
		os << "stack trace:";

		std::vector<void *> addresses(max_trace + 1);
		const auto num_addresses = ::backtrace(addresses.data(), addresses.size());
		if (num_addresses <= 0) {
			os << " empty or corrupted\n";
			return os;
		}
		os << "\n";

		char ** symbols = backtrace_symbols(addresses.data(), num_addresses);
		char * function_name = static_cast<char *>(malloc(max_func_name_size + 1));

		using iterator = std::remove_const<decltype(num_addresses)>::type;
		for (iterator i = 0; i < num_addresses; ++i) {
			os << " "; // indentation

			// find name and offset
			char * name = nullptr;
			char * offset = nullptr;
			char * end = nullptr;
			for (char * p = symbols[i]; *p && !end; ++p) {
				if (*p == '(')
					name = p + 1;
				if (*p == '+')
					offset = p + 1;
				if (*p == ')')
					end = p;
			}

			// print entry
			if (name && offset && end) {
				*(offset - 1) = '\0';
				*end = '\0';

				// c++ demangle
				int status = 0;
				size_t size = max_func_name_size;
				function_name = abi::__cxa_demangle(name, function_name, &size, &status);
				if (status == 0) {
					os << name << ": " << function_name << "\n";
				} else {
					// demangle error
					os << name << "\n";
				}
			} else {
				os << symbols[i] << "\n";
			}
		}

		free(function_name);
		free(symbols);
		return os;
	}
};

std::ostream & operator<<(std::ostream & os, const stacktrace & s) { return s(os); }
}

template <typename T> T h(T * p)
{
	std::cout << utils::stacktrace{} << "\n";
	return *p;
}

int f(int * p) { return h(p); }

int g(int * p) { return f(p); }

int main(int, char **)
{
	int a = 99;
	int b = g(&a);

	std::cout << "a=" << a << ", b=" << b << "\n";
	return 0;
}
