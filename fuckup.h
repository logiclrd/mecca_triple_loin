#ifndef FUCKUP_H
#define FUCKUP_H

#include "types.h"

NORETURN void explode(const char *message);
NORETURN void complain(int code, const char *message, const char *line, int line_number, int column);
const char *error_code_to_string(int code);

#endif /* FUCKUP_H */

