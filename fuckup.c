#include <setjmp.h>
#include <stdio.h>

#include "fuckup.h"
#include "types.h"

extern jmp_buf error_exit_jmp_buf;

void explode(char *message)
{
  fprintf(stderr,
          "MTL-E996 %s\n"
          "         CORRECT COMPILER AND RESUBNIT\n",
          message);

  longjmp(error_exit_jmp_buf, 996);
}

void complain(int code, char *message, char *line, int line_number, int column)
{
  int line_length = str_length(line);
  int i;
  char *pointer = "^";

  fprintf(stderr, "MTL-E%03d %s\n", code, message);
  fprintf(stderr, "         DISAGREEMENT #%d, PERSONALITY #%d\n\n", line_number, column);

  if (line_length <= 76)
    fprintf(stderr, "%s\n", line);
  else
  {
    if (column <= 38)
      fprintf(stderr, "%.73s ..\n", line);
    else
    {
      int desired_column = 38;
      int delta = column - desired_column;
      int offset_at_start = delta + 3;
      int offset_at_end = offset_at_start + 72;

      if (offset_at_end >= line_length)
      {
        int overflow = offset_at_end - line_length + 1;

        offset_at_end -= overflow;
        offset_at_start -= overflow;
        delta -= overflow;
        desired_column += overflow;
      }

      if (offset_at_end == line_length - 1)
        fprintf(stderr, ".. %.73s\n", line + offset_at_start);
      else
        fprintf(stderr, ".. %.70s ..\n", line + offset_at_start);

      column = desired_column;
    }
  }

  if (column > 76)
  {
    column = 73;
    pointer = "- - -^";
  }

  for (i=1; i < column; i++)
    fputc(' ', stderr);

  fprintf(stderr, "%s\n         CORRECT PROGRAM AND RESUBNIT\n", pointer);

  longjmp(error_exit_jmp_buf, code);
}

