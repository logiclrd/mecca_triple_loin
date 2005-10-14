#include <setjmp.h>
#include <stdio.h>

#include "fuckup.h"
#include "types.h"

extern jmp_buf error_exit_jmp_buf;

extern bool strict_error_message_format;

void explode(const char *message)
{
  if (strict_error_message_format)
    fprintf(stderr,
            "ICL996I %s\n"
            "        ON THE WAY TO MECCA\n"
            "        CORRECT COMPILER AND RESUBNIT\n",
            message);
  else
    fprintf(stderr,
            "MTL-E996 %s\n"
            "         CORRECT COMPILER AND RESUBNIT\n",
            message);

  longjmp(error_exit_jmp_buf, 996);
}

void complain(int code, const char *message, const char *line, int line_number, int column)
{
  if (strict_error_message_format)
    fprintf(stderr,
            "ICL%.3dI %s\n"
            "        ON THE WAY TO STATEMENT %d\n"
            "        CORRECT SOURCE AND RESUBNIT\n",
            code, message, line_number);
  else
  {
    int line_length = line ? str_length(line) : 0;
    int i;
    char *pointer = line ? "^\n" : "\n";

    if (str_equal(message, "%s"))
      fprintf(stderr, "MTL-E%03d %s\n", code, line ? line : "(NULL)");
    else
      fprintf(stderr, "MTL-E%03d %s\n", code, message);
    if (line)
      fprintf(stderr, "         DISAGREEMENT #%d, PERSONALITY #%d\n\n", line_number, column);

    if (line_length <= 76)
    {
      if (line)
        fprintf(stderr, "%s\n", line);
    }
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
      pointer = "- - -^\n";
    }

    for (i=1; i < column; i++)
      fputc(' ', stderr);

    fprintf(stderr, "%s         CORRECT PROGRAM AND RESUBNIT\n", pointer);
  }

  longjmp(error_exit_jmp_buf, code);
}

typedef struct sCode
{
  int Value;
  const char *Message;
} Code;

/* note: not all of these are used */
Code codes[] =
{
  {   0, "%s" },
  {  17, "DO YOU EXPECT ME TO UNDERSTAND THIS?!:\n\n%s\n\n" },
  {  79, "THE PROGRAMMER IS INSUFFICIENTLY POLITE" },
  {  99, "THE PROGRAMMER IS OVERLY POLITE" },
  { 123, "PROGRAM HAS DISAPPEARED INTO THE BLACK LAGOON." },
  { 129, "THE PROGRAM FELL OFF THE LINE" },
  { 139, "THE PROGRAM IS SENILE" },
  { 182, "THE PROGRAMMER IS FORGETFUL" },
  { 197, "THE LABEL LACKS ADHESION" },
  { 200, "YOU CANNOT SEE WHAT DOES NOT EXIST" },
  { 240, "YOU CAN'T CREATE NOTHING" },
  { 241, "NUMBERS MAY NOT BE SUBDIVIDED" },
  { 275, "THE PROGRAMMER HAS JOHNNY COCHRANNED A ONESPOT" },
  { 436, "THROW STICK BEFORE RETRIEVING" },
  { 533, "THE PROGRAMMER HAS JOHNNY COCHRANNED A TWOSPOT" },
  { 562, "DATUM SOURCE HAS RUN DRY" },
  { 579, "THE INPUT IS SEMANTICALLY IMPAIRED" },
  { 621, "NOTHING HAPPENS WHEN YOU DO SOMETHING 0 TIMES" },
  { 632, "YOU WENT TOO FAR THIS TIME" },
  { 633, "YOU FELL TOO FAR THIS TIME" },
  { 774, "COMPILER-GENERATED ENTROPY INJECTION" },
  { 778, "I TOOK THE WRONG PATH" },
  /* C-INTERCAL extensions: */
  { 111, "COMPLY WITH THE STANDARD YOU TURKEY" },
  { 127, "SYSTEM CORE GOT LOST" },
  { 222, "YOU CAN'T HIDE AND YOU MAY NO LONGER RUN" },
  { 333, "I HAVE A HEADACHE" },
  { 444, "DO NOT INVITE EVIL" },
  { 555, "QUANTUM PROBABILITY FIELDS ARE PROHIBITED" },
  { 666, "SUBSTRATE COLLAPSED UNDER WEIGHT OF SEQUENCE" },
  { 777, "UNABLE TO CONJURE FROM THE VOID THE GURU'S INTENTION" },
  { 888, "OPERATION DAMAGED BY HOSTILE ENVIRONMENT" },
  { 999, "STRUCTURAL INTEGRITY COMPROMISED" },
  { 998, "SYSTEMATIC NOMENCLATURE FAILURE" },
  { 997, "YOU ARE ON THE WRONG BASE, ENSIGN" },
  /* MTL extensions: */
  {  33, "THE IMPROBABLE IS WHAT USUALLY HAPPENS" },
  { 300, "CANNOT STUFF DIMENSIONS INTO A BOX" },
  { 583, "THE ARRAY WAS NOT PUT THERE" },

  /* end of list: */
  { -1, NULL }
};

const char *error_code_to_string(int code)
{
  int i;

  for (i=0; codes[i].Value >= 0; i++)
    if (codes[i].Value == code)
      return codes[i].Message;

  return "BAD LUCK IN MODULE F-E-C-T-S";
}
