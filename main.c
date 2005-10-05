#include <setjmp.h>

#include "parser.h"
#include "types.h"

jmp_buf error_exit_jmp_buf;

bool strict_error_message_format = false;

int main()
{
  StatementList program;

  int error_code = setjmp(error_exit_jmp_buf);

  if (error_code != 0)
    return error_code;

  program = parse(stdin);

  return 0;
}

