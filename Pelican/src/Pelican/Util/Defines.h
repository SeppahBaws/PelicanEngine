#pragma once

#define ENABLE_ASSERTIONS // TODO: move to premake file
#ifdef ENABLE_ASSERTIONS
#include <iostream>

#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __asm { int 3 }
#endif // _MSC_VER

#define ASSERT(expr) \
	if (!(expr)) {\
		ReportAssertionFailure(#expr, "", __FILE__, __LINE__); \
		debugBreak(); \
	}

#define ASSERT_MSG(expr, msg) \
	if (!(expr)) { \
		ReportAssertionFailure(#expr, msg, __FILE__, __LINE__); \
		debugBreak(); \
	}

inline void ReportAssertionFailure(const char* expression, const char* msg, const char* file, int line)
{
	std::cerr << "Assertion failed: " << expression << " -- message: '" << msg << "' in file " << file << " at line " << line << std::endl;
}

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr)
#endif // ENABLE_ASSERTIONS
