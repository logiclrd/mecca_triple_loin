#include <setjmp.h>

jmp_buf error_exit_jmp_buf;

int main()
{
  int error_code = setjmp(error_exit_jmp_buf);

  if (error_code != 0)
    return error_code;


  return 0;
}

