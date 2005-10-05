#ifndef TYPES_H
#define TYPES_H

typedef enum { false, true } bool;

typedef unsigned int uint;
typedef unsigned short ushort;

#define alloc(type) ((type *)malloc(sizeof(type)))

void *malloc(size_t);
void free(void *);

#include <string.h>

#define str_length(a) ((int)strlen(a))
#define str_equal(a, b) strcmp(a, b)
#define substr_equal(a, b, n) strncmp(a, b, n)
#ifdef WIN32
# define str_equal_nocase(a, b) stricmp(a, b)
# define substr_equal_nocase(a, b, n) strnicmp(a, b, n)
#else
# define str_equal_nocase(a, b) strcasecmp(a, b)
# define substr_equal_nocase(a, b, n) strncasecmp(a, b, n)
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

