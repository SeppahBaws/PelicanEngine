#pragma once

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef float		f32;
typedef double		f64;


#ifdef PELICAN_DEBUG
#define ENABLE_ASSERTIONS
#endif

#ifdef ENABLE_ASSERTIONS
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

#include <iostream>
inline void ReportAssertionFailure(const char* expression, const char* msg, const char* file, int line)
{
	std::cerr << "Assertion failed: " << expression << " -- message: '" << msg << "' in file " << file << " at line " << line << std::endl;
}

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr, msg)
#endif // ENABLE_ASSERTIONS

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
