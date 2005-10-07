#include <ctype.h>
#include <stdio.h>

#include "fuckup.h"
#include "types.h"

static int decode(uchar *digit, int digit_length)
{
  switch (toupper(digit[0]))
  {
    case 'E': // EIGHT
    {
      if ((digit_length == 5) && substr_equal_nocase(digit, "EIGHT", 5))
        return 8;

      break;
    }
    case 'F': // FIVE, FOUR
    {
      if ((digit_length == 4) && substr_equal_nocase(digit, "FIVE", 4))
        return 5;

      if ((digit_length == 4) && substr_equal_nocase(digit, "FOUR", 4))
        return 4;

      break;
    }
    case 'N': // NINE, NINER
    {
      if ((digit_length == 4) && substr_equal_nocase(digit, "NINE", 4))
        return 9;

      if ((digit_length == 5) && substr_equal_nocase(digit, "NINER", 5))
        return 9;

      break;
    }
    case 'O': // OH, ONE
    {
      if ((digit_length == 2) && substr_equal_nocase(digit, "OH", 2))
        return 0;

      if ((digit_length == 3) && substr_equal_nocase(digit, "ONE", 3))
        return 1;

      break;
    }
    case 'S': // SEVEN, SIX
    {
      if ((digit_length == 5) && substr_equal_nocase(digit, "SEVEN", 5))
        return 7;

      if ((digit_length == 3) && substr_equal_nocase(digit, "SIX", 3))
        return 6;

      break;
    }
    case 'T': // THREE, TWO
    {
      if ((digit_length == 5) && substr_equal_nocase(digit, "THREE", 5))
        return 3;

      if ((digit_length == 3) && substr_equal_nocase(digit, "TWO", 3))
        return 2;

      break;
    }
  }

  return -1;
}

uint text_in()
{
  uchar digit[30];
  uint accumulator = 0;

  while (true)
  {
    int digit_length = 0;
    int digit_value;

    do
      digit[0] = getc(stdin);
    while (isspace(digit[0]));

    do
      digit[++digit_length] = getc(stdin);
    while (!isspace(digit[digit_length]));

    digit_value = decode(digit, digit_length);

    if ((digit_value < 0)
     || (accumulator > 429496729)
     || ((accumulator == 429496729) && (digit_value >= 6)))
      complain(579, error_code_to_string(579), NULL, 0, 0);

    accumulator = (accumulator * 10) + digit_value;

    if (digit[digit_length] == '\n')
      break;
  }

  return accumulator;
}

static const char *numerals_uppercase[] =
{
  { "I" },
  { "V" },
  { "X" },
  { "L" },
  { "C" },
  { "D" },
  { "M" }
};

static const char *numerals_lowercase[] =
{
  { "i" },
  { "v" },
  { "x" },
  { "l" },
  { "c" },
  { "d" },
  { "m" }
};

#define I 0
#define V 1
#define X 2
#define L 3
#define C 4
#define D 5
#define M 6

static void append_number_part(int part, bool use_overbar, bool use_lowercase, char *overbar_chars, char *numeral_chars)
{
  const char **numeral;
  char *overbar;
  int denominations[7] = { 1, 5, 10, 50, 100, 500, 1000 };
  int i, j;

  if (use_overbar)
    overbar = "_";
  else
    overbar = " ";

  if (use_lowercase)
    numeral = numerals_lowercase;
  else
    numeral = numerals_uppercase;

  for (i = 6; i >= 0; i -= 2)
  {
    while (part >= denominations[i])
    {
      strcat(overbar_chars, overbar);
      strcat(numeral_chars, numeral[i]);

      part -= denominations[i];
    }

    for (j = 0; j < i; j++)
    {
      if (10 * denominations[j] > denominations[i])
        break;

      if (part >= denominations[i] - denominations[j])
      {
        strcat(overbar_chars, overbar);
        strcat(numeral_chars, numeral[j]);
        strcat(overbar_chars, overbar);
        strcat(numeral_chars, numeral[i]);

        part -= (denominations[i] - denominations[j]);
      }
    }

    if (i >= 2)
    {
      if (part >= denominations[i - 1])
      {
        strcat(overbar_chars, overbar);
        strcat(numeral_chars, numeral[i - 1]);

        while (part >= denominations[i - 1] + denominations[i - 2])
        {
          strcat(overbar_chars, overbar);
          strcat(numeral_chars, numeral[i - 2]);

          part -= denominations[i - 2];
        }

        part -= denominations[i - 1];
      }

      if (part >= denominations[i - 1] - denominations[i - 2])
      {
        strcat(overbar_chars, overbar);
        strcat(numeral_chars, numeral[i - 2]);
        strcat(overbar_chars, overbar);
        strcat(numeral_chars, numeral[i - 1]);

        part -= (denominations[i - 1] - denominations[i - 2]);
      }
    }
  }
}

#undef I
#undef V
#undef X
#undef L
#undef C
#undef D
#undef M

void text_out(uint value)
{
  char overbar_chars[30] = { 0 };
  char numeral_chars[30] = { 0 };

  if (value == 0)
  {
    printf("\n_\n \n", value);
    return;
  }

  if (value >= 4000000000)
  {
    append_number_part(value / 1000000000, true, true, overbar_chars, numeral_chars);
    value %= 1000000000;
  }

  if (value >= 4000000)
  {
    append_number_part(value / 1000000, false, true, overbar_chars, numeral_chars);
    value %= 1000000;
  }

  if (value >= 4000)
  {
    append_number_part(value / 1000, true, false, overbar_chars, numeral_chars);
    value %= 1000;
  }

  if (value > 0)
    append_number_part(value, false, false, overbar_chars, numeral_chars);

  printf("\n%s\n%s\n", overbar_chars, numeral_chars);
}

static int last_char_in = 0;

void binary_in(uchar *target, int size, int stride)
{
  while (size > 0)
  {
    int char_in = fgetc(stdin);

    int delta = char_in - last_char_in;

    *target = (uchar)delta;

    last_char_in = char_in;

    target += stride, size--;
  }
}

// oh boy does this ever render the input useless!
void binary_skip_in(int size)
{
  while (size > 0)
  {
    last_char_in = fgetc(stdin);
    size--;
  }
}

static void fputc_backwards(int bits, FILE *out)
{
  // swap groups of 1 bit
  bits = ((bits & 0xAA) >> 1) | ((bits & 0x55) << 1);
  // swap groups of 2 bits
  bits = ((bits & 0xCC) >> 2) | ((bits & 0x33) << 2);
  // swap groups of 4 bits
  bits = ((bits & 0xF0) >> 4) | ((bits & 0x0F) << 4);

  fputc(bits, out);
}

static int last_char_out = 0;

void binary_out(uchar *source, int size, int stride)
{
  while (size > 0)
  {
    int delta = *source;

    int char_out = last_char_out - delta;

    fputc_backwards((uchar)char_out, stdout);

    last_char_out = char_out;

    source += stride, size--;
  }
}

