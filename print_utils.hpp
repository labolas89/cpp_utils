#pragma once
#ifndef __PRINT_UTILS_H__
#define __PRINT_UTILS_H__


#define PRINT_FUNCTION              1
#define DEBUG_LUJ                  0

#if PRINT_FUNCTION

//-Wno-variadic-macros
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#ifdef _WIN32 // for embeded
#if DEBUG_LUJ
#if 0 // filename print
#define dprint(fmt, arg...)  \
                printf("<%s:%u>: " ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET, \
                __FILE__, __LINE__, __VA_ARGS__)
#else // small print
#define dprint(fmt, ...)  \
                printf("<%s>: " ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET, \
                __FUNCTION__, __VA_ARGS__)
#endif
#else // ~DEBUG_LUJ
#define dprint(fmt, ...) void(0)
#endif // ~DEBUG_LUJ

#define eprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_RED fmt ANSI_COLOR_RESET, \
            __FUNCTION__, __VA_ARGS__)

#define wprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_MAGENTA fmt ANSI_COLOR_RESET, \
            __FUNCTION__, __VA_ARGS__)

#define iprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_GREEN fmt ANSI_COLOR_RESET, \
            __FUNCTION__, __VA_ARGS__)
#else // _WIN32 // for pc
#if DEBUG_LUJ
#if 0 // filename print
#define dprint(fmt, arg...)  \
                printf("<%s:%u>: " ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET, \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else // small print
#define dprint(fmt, ...)  \
                printf("<%s>: " ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET, \
                __FUNCTION__, ##__VA_ARGS__)
#endif
#else // ~DEBUG_LUJ
#define dprint(fmt, ...) void(0)
#endif // ~DEBUG_LUJ

#define eprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_RED fmt ANSI_COLOR_RESET, \
            __FUNCTION__, ##__VA_ARGS__)

#define wprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_MAGENTA fmt ANSI_COLOR_RESET, \
            __FUNCTION__, ##__VA_ARGS__)

#define iprint(fmt, ...)  \
            printf("<%s>: " ANSI_COLOR_GREEN fmt ANSI_COLOR_RESET, \
            __FUNCTION__, ##__VA_ARGS__)
#endif

#else // ~PRINT_FUNCTION
#define dprint(fmt, ...) void(0)
#define eprint(fmt, ...) void(0)
#define wprint(fmt, ...) void(0)
#define iprint(fmt, ...) void(0)
#endif // ~PRINT_FUNCTION

constexpr bool static_string_equal(char const * a, char const * b) {
	return *a == *b && (*a == '\0' || static_string_equal(a + 1, b + 1));
}

constexpr bool static_string_find(char const* buffer, char const* search) {
	while (*buffer)
	{
		if (static_string_equal(buffer++, search))
			return true;
	}
	return false;
}

#endif // !__PRINT_UTILS_H__

