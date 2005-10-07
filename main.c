#include <setjmp.h>

#include "interpret.h"
#include "parser.h"
#include "types.h"

jmp_buf error_exit_jmp_buf;

bool strict_error_message_format = false;

int main(int argc, char *argv[])
{
  StatementList program;
  FILE *input;

  int error_code = setjmp(error_exit_jmp_buf);

  if (error_code != 0)
    return error_code;

  if (argc <= 1)
  {
    fprintf(stderr, "yeah, your mom.\n");
    return 1;
  }

  input = fopen(argv[1], "rb");

  program = parse(input);

  interpret(program);

  return 0;
}

