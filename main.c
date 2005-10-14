#include <setjmp.h>
#include <stdlib.h>

#ifdef WIN32
void GetSystemTimeAsFileTime(__int64 *lpSystemTimeAsFileTime);
#else
# include <sys/time.h>
#endif /* WIN32 */

#include "interpret.h"
#include "compile.h"
#include "parser.h"
#include "types.h"

jmp_buf error_exit_jmp_buf;

bool strict_error_message_format = false;
bool strict_call_stack_size = true;
bool cheat_for_syslib_functions = true;
bool wimp_mode = false;
bool emit_intermediate_code = false;

void seed_rand()
{
#ifdef WIN32

#else
  struct timeval tv;

  gettimeofday(&tv, NULL);

  srand(time(NULL) ^ tv.tv_usec);
#endif /* WIN32 */
}

int main(int argc, char *argv[])
{
  bool interpreting = false;
  bool end_of_switches = false;
  bool bad = false;
  int i;

  int error_code = setjmp(error_exit_jmp_buf);

  if (error_code != 0)
    return error_code;

  seed_rand();

  if (argc <= 1)
  {
    fprintf(stderr, "yeah, your mom.\n");
    return 1;
  }

  for (i=1; i < argc; i++)
  {
    char *arg = argv[i];

    if (((arg[0] == '-') || (arg[0] == '+')) && !end_of_switches)
    {
      bool on = (arg[0] == '+');

      if (str_length(arg) == 2)
        switch (arg[1])
        {
          case '-':
          case '+': end_of_switches = true;           break;

          case 'E': strict_error_message_format = on; break;
          case 'C': strict_call_stack_size = on;      break;
          case 'D': cheat_for_syslib_functions = !on; break;
          case 'w': wimp_mode = on;                   break;
          case 'S':
            strict_error_message_format = strict_call_stack_size = on;
            cheat_for_syslib_functions = wimp_mode = !on;
            break;

          case 'i': interpreting = on;                break;
          case 'c': emit_intermediate_code = on;      break;

          default:
            fprintf(stderr, "unknown option: %s\n", arg);
            bad = true;
        }
      else
      {
        if (arg[1] != arg[0])
        {
          if ((arg[1] == '-') || (arg[1] == '+'))
            on = (rand() > (RAND_MAX >> 1));
          else
          {
            fprintf(stderr, "invalid option syntax: %s\n", arg);
            bad = true;

            continue;
          }
        }

        arg += 2;

        if (str_equal_nocase(arg, "strict-errors"))
          strict_error_message_format = on;
        else if (str_equal_nocase(arg, "strict-call-stack"))
          strict_call_stack_size = on;
        else if (str_equal_nocase(arg, "no-cheating"))
          cheat_for_syslib_functions = !on;
        else if (str_equal_nocase(arg, "wimp-mode"))
          wimp_mode = on;
        else if (str_equal_nocase(arg, "standard"))
        {
          strict_error_message_format = strict_call_stack_size = on;
          cheat_for_syslib_functions = wimp_mode = !on;
        }
        else if (str_equal_nocase(arg, "interpret"))
          interpreting = on;
        else if (str_equal_nocase(arg, "compile"))
          interpreting = !on;
        else if (str_equal_nocase(arg, "emit-intermediate"))
          emit_intermediate_code = on;
        else
        {
          fprintf(stderr, "unknown option: %s\n", arg - 2);
          bad = true;
        }
      }

      continue;
    }

    if (!bad)
    {
      FILE *input = fopen(arg, "rb");

      StatementList program = parse(input);

      fclose(input);

      if (interpreting)
        interpret(program);
      else
        compile(program, arg);
    }
  }

  return bad;
}

