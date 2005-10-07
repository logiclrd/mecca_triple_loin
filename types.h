#ifndef TYPES_H
#define TYPES_H

typedef enum { false, true } bool;

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define alloc(type) ((type *)malloc(sizeof(type)))

void *malloc(size_t);
void free(void *);

#include <string.h>

#define str_copy(a, b) strcpy(a, b)
#define substr_copy(a, b, n) (memcpy(a, b, n), a[n] = 0)
#define str_length(a) ((int)strlen(a))
#define str_equal(a, b) (0 == strcmp(a, b))
#define substr_equal(a, b, n) (0 == strncmp(a, b, n))
#ifdef WIN32
# define str_equal_nocase(a, b) (0 == stricmp(a, b))
# define substr_equal_nocase(a, b, n) (0 == strnicmp(a, b, n))
#else
# define str_equal_nocase(a, b) (0 == strcasecmp(a, b))
# define substr_equal_nocase(a, b, n) (0 == strncasecmp(a, b, n))
#endif /* WIN32 */

#ifndef NULL
# define NULL 0
#endif

#if defined(_MSC_VER)
# define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
# define NORETURN __attribute__(__noreturn__)
#else
# define NORETURN
#endif

#endif /* TYPES_H */

