#ifndef FUCKUP_H
#define FUCKUP_H

#include "types.h"

NORETURN void explode(char *message);
NORETURN void complain(int code, char *message, char *line, int line_number, int column);
const char *error_code_to_string(int code);

#endif /* FUCKUP_H */

