#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>
#include <stddef.h>
#include <allwnr_f133.h>

#define SOC_D1H 'h'
#define SOC_D1S 's'

#ifndef SOC
#define SOC SOC_D1H
#endif

#define PLATFORM_SIPEED     's'
#define PLATFORM_DEVTERM    'd'

#ifndef PLATFORM
#define PLATFORM PLATFORM_SIPEED
#endif

#ifndef VERSION_GIT
#define VERSION_GIT "NA"
#endif

#define __IO volatile
#define LIKELY(x)          __builtin_expect(!!(x), 1)
#define UNLIKELY(x)        __builtin_expect(!!(x), 0)
#define ARRAY_SIZE(x)      ((sizeof(x)/sizeof(x[0])))
#define BV(x)              (1UL << (x))
#define ABS(x)             (((x)>0)?(x):(-(x)))
#define MAX(x,y)           ((x)>(y)?(x):(y))
#define MIN(x,y)           ((x)>(y)?(y):(x))
#define CEIL_DIV(a,b) (((a) + (b) - 1) / (b))
#define SWAP(a, b) do { \
	typeof(a) tmp = b; \
	b = a;             \
	a = tmp;           \
} while (0)

#define PACK               __attribute__ ((packed))

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)

/* These can't be used after statements in c89. */
#ifdef __COUNTER__
	#define STATIC_ASSERT(e,m) \
		{ enum { ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(!!(e)) }; }
#else
	/* This can't be used twice on the same line so ensure if using in headers
	 * that the headers are not included twice (by wrapping in #ifndef...#endif)
	 * Note it doesn't cause an issue when used on same line of separate modules
	 * compiled with gcc -combine -fwhole-program.  */
	#define STATIC_ASSERT(e,m) \
		{ enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }; }
#endif

#define MAGIC_KEY  0xDEADBEEF

extern uint32_t SystemCoreClock;
extern uint32_t SystemAPB1Clock;
extern uint32_t SystemAPB2Clock;

#endif

