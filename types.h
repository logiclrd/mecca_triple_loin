#ifndef TYPES_H
#define TYPES_H


typedef enum { false, true } bool;

#ifdef __GNUC__
# ifndef __USE_MISC
#  define __USE_MISC
# endif
# include <sys/types.h>
#else
 typedef unsigned int uint;
 typedef unsigned short ushort;
#endif
typedef unsigned char uchar;

#define alloc(type) ((type *)malloc(sizeof(type)))

void *malloc(size_t);
void free(void *);

#include <string.h>

#define str_copy(a, b) strcpy((char *)(a), (char *)(b))
#define substr_copy(a, b, n) (memcpy(a, b, n), a[n] = 0)
#define str_length(a) ((int)strlen((char *)(a)))
#define str_equal(a, b) (0 == strcmp((char *)(a), (char *)(b)))
#define substr_equal(a, b, n) (0 == strncmp((char *)(a), (char *)(b), n))
#ifdef WIN32
# define str_equal_nocase(a, b) (0 == stricmp((char *)(a), (char *)(b)))
# define substr_equal_nocase(a, b, n) (0 == strnicmp((char *)(a), (char *)(b), n))
#else
# define str_equal_nocase(a, b) (0 == strcasecmp((char *)(a), (char *)(b)))
# define substr_equal_nocase(a, b, n) (0 == strncasecmp((char *)(a), (char *)(b), n))
#endif /* WIN32 */

#ifndef NULL
# define NULL 0
#endif

#if defined(_MSC_VER)
# define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
# define NORETURN __attribute__((__noreturn__))
#else
# define NORETURN
#endif

#endif /* TYPES_H */

