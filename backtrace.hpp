//
// Copyright (c) 2015, Mario Konrad <mario.konrad@gmx.net>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the software nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL Mario Konrad BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef __UTILS__BACKTRACE__HPP__
#define __UTILS__BACKTRACE__HPP__

#include <ostream>
#include <vector>
#include <execinfo.h>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace utils
{
/// @cond DEV
namespace detail
{
template <class Trait> struct basic_stacktrace {
	std::ostream & operator()(std::ostream & os, const int max_trace = 31) const
	{
		os << "stack trace:";

		std::vector<void *> addresses(max_trace + 1);
		const auto num_addresses = ::backtrace(addresses.data(), addresses.size());
		if (num_addresses <= 0) {
			os << " empty or corrupted\n";
			return os;
		}
		os << "\n";

		// this call allocates memory (malloc)
		char ** symbols = backtrace_symbols(addresses.data(), num_addresses);

		using iterator = std::remove_const<decltype(num_addresses)>::type;
		for (iterator i = 0; i < num_addresses; ++i) {
			os << "- "; // indentation
			Trait::render(os, symbols[i]);
		}

		free(symbols);
		return os;
	}
};

template <class Trait>
std::ostream & operator<<(std::ostream & os, const basic_stacktrace<Trait> & s)
{
	return s(os);
}

struct raw_trace {
	static void render(std::ostream & os, char * symbol) { os << symbol << "\n"; }
};

template <class Format> struct format_trace {
	static void render(std::ostream & os, char * symbol)
	{
		// find name and offset
		char * name = nullptr;
		char * offset = nullptr;
		char * end = nullptr;
		for (char * p = symbol; *p && !end; ++p) {
			if (*p == '(')
				name = p + 1;
			if (*p == '+')
				offset = p + 1;
			if (*p == ')')
				end = p;
		}

		// print entry
		if (name && offset && end) {
			*(name - 1) = '\0';
			*(offset - 1) = '\0';
			*end = '\0';

			Format::format(os, symbol, name, offset);
		} else {
			os << symbol << "\n";
		}
	}
};

struct simple_format {
	static void format(
		std::ostream & os, const char * org, const char * name, const char * offset)
	{
		os << org << ": " << name << " +" << offset << "\n";
	}
};

#ifdef __GNUC__
/// Renders the mangled function name using GCC demangling.
///
/// Does not reuse any allocated memory, for every name to be demangled, a memory
/// allocation/deallocation occurrs.
struct demangling_format {
	static void format(
		std::ostream & os, const char * org, const char * name, const char * offset)
	{
		// c++ demangle
		int status = 0;

		// this call allocates memory (malloc)
		char * function_name = abi::__cxa_demangle(name, nullptr, nullptr, &status);
		if (status == 0) {
			os << org << ": " << function_name << " +" << offset << "\n";
		} else {
			// fallback, demangle error
			os << org << ": " << name << " +" << offset << "\n";
		}
		free(function_name);
	}
};
#endif
}
/// @endcond

using stacktrace = detail::basic_stacktrace<detail::format_trace<
#ifdef __GNUC__
	detail::demangling_format
#else
	detail::simple_format
#endif
	>>;
}

#endif
