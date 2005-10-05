#include <ctype.h>
#include <stdio.h>

#include "program.h"
#include "fuckup.h"
#include "types.h"

typedef enum eTokenType
{
  TokenType_InvalidToken,
  TokenType_UnmatchedChar,
  // symbols:
  TokenType_007,
  TokenType_Ears,
  TokenType_Gets,
  TokenType_Hybrid,
  TokenType_Mesh,
  TokenType_Mingle,
  TokenType_OneSpot,
  TokenType_ParenLeft,
  TokenType_ParenRight,
  TokenType_Select,
  TokenType_Spark,
  TokenType_Splat,
  TokenType_Tail,
  TokenType_TwoSpot,
  // words:
  TokenType_Abstain,
  TokenType_By,
  TokenType_Come,
  TokenType_Do,
  TokenType_Forget,
  TokenType_From,
  TokenType_Give,
  TokenType_Ignore,
  TokenType_In,
  TokenType_Next,
  TokenType_Not,
  TokenType_Out,
  TokenType_Please,
  TokenType_Read,
  TokenType_Reinstate,
  TokenType_Remember,
  TokenType_Resume,
  TokenType_Retrieve,
  TokenType_Stash,
  TokenType_Sub,
  TokenType_Up,
  TokenType_Write,
  // dynamic:
  TokenType_Number,
} TokenType;

typedef struct sToken
{
  TokenType Type;
  int Row, Column;
  int Value;
} Token;

static Token pull_token(char **buf_ptr, int *row, int *column)
{
#define QUO '\''
  Token ret;

  while (isspace(**buf_ptr))
  {
    switch (**buf_ptr)
    {
      case '\t': /* horizontal tab  */ *column = 8 * ((*column + 7) >> 3); break;
      case '\n': /* newline         */ ++*row; *column = 1;                break;
      case 11:   /* vertical tab??  */                                     break;
      case 12:   /* page break      */ *row = 60 * ((*row + 59) / 60);     break;
      case '\r': /* carriage return */ *column = 1;                        break;
      case ' ':  /* space           */ ++*column;                          break;
    }

    ++*buf_ptr;
  }

  ret.Value = **buf_ptr;
  ret.Type = TokenType_UnmatchedChar;
  ret.Row = *row;
  ret.Column = *column;

  switch (**buf_ptr)
  {
    // symbols:
    case '%': ++*buf_ptr; ret.Type = TokenType_007;         break;
    case '"': ++*buf_ptr; ret.Type = TokenType_Ears;        break;
    case '<':
      if ((*buf_ptr)[1] == '-')
      {
        (*buf_ptr) += 2;
        ret.Type = TokenType_Gets;
      }
      break;
    case ';': ++*buf_ptr; ret.Type = TokenType_Hybrid;      break;
    case '#': ++*buf_ptr; ret.Type = TokenType_Mesh;        break;
    case '/':
    case '$': ++*buf_ptr; ret.Type = TokenType_Mingle;      break;
    case '.': ++*buf_ptr; ret.Type = TokenType_OneSpot;     break;
    case '(': ++*buf_ptr; ret.Type = TokenType_ParenLeft;   break;
    case ')': ++*buf_ptr; ret.Type = TokenType_ParenRight;  break;
    case '~': ++*buf_ptr; ret.Type = TokenType_Select;      break;
    case QUO: ++*buf_ptr; ret.Type = TokenType_Spark;       break;
    case '*': ++*buf_ptr; ret.Type = TokenType_Splat;       break;
    case ',': ++*buf_ptr; ret.Type = TokenType_Tail;        break;
    case ':': ++*buf_ptr; ret.Type = TokenType_TwoSpot;     break;
    // words:
    case 'a': case 'A':
      if (substr_equal_nocase(*buf_ptr, "ABSTAIN", 7))
      {
        (*buf_ptr) += 7;
        ret.Type = TokenType_Abstain;
      }
      break;
    case 'b': case 'B':
      if (((*buf_ptr)[1] == 'y') || ((*buf_ptr)[1] == 'Y'))
      {
        (*buf_ptr) += 2;
        ret.Type = TokenType_By;
      }
      break;
    case 'c': case 'C':
      if (substr_equal_nocase(*buf_ptr, "COME", 4))
      {
        (*buf_ptr) += 4;
        ret.Type = TokenType_Come;
      }
      break;
    case 'd': case 'D':
      if (((*buf_ptr)[1] == 'o') || ((*buf_ptr)[1] == 'O'))
      {
        (*buf_ptr) += 2;
        ret.Type = TokenType_Do;
      }
      break;
    case 'f': case 'F':
      if (substr_equal_nocase(*buf_ptr, "FORGET", 6))
      {
        (*buf_ptr) += 6;
        ret.Type = TokenType_Forget;
      }
      else if (substr_equal_nocase(*buf_ptr, "FROM", 4))
      {
        (*buf_ptr) += 4;
        ret.Type = TokenType_From;
      }
      break;
    case 'g': case 'G':
      if (substr_equal_nocase(*buf_ptr, "GIVE", 4))
      {
        (*buf_ptr) += 4;
        ret.Type = TokenType_Give;
      }
      break;
    case 'i': case 'I':
      if (substr_equal_nocase(*buf_ptr, "IGNORE", 6))
      {
        (*buf_ptr) += 6;
        ret.Type = TokenType_Ignore;
      }
      else if (((*buf_ptr)[1] == 'n') || ((*buf_ptr)[1] == 'N'))
      {
        (*buf_ptr) += 2;
        ret.Type = TokenType_In;
      }
      break;
    case 'n': case 'N':
      if (substr_equal_nocase(*buf_ptr, "NEXT", 4))
      {
        (*buf_ptr) += 4;
        ret.Type = TokenType_Next;
      }
      else if (substr_equal_nocase(*buf_ptr, "NOT", 3)
            || substr_equal_nocase(*buf_ptr, "N'T", 3))
      {
        (*buf_ptr) += 3;
        ret.Type = TokenType_Not;
      }
      break;
    case 'o': case 'O':
      if (substr_equal_nocase(*buf_ptr, "OUT", 3))
      {
        (*buf_ptr) += 3;
        ret.Type = TokenType_Out;
      }
      break;
    case 'p': case 'P':
      if (substr_equal_nocase(*buf_ptr, "PLEASE", 6))
      {
        (*buf_ptr) += 6;
        ret.Type = TokenType_Please;
      }
      break;
    case 'r': case 'R':
      if (((*buf_ptr)[1] != 'e') && ((*buf_ptr)[1] != 'E'))
        break;
      if (substr_equal_nocase(*buf_ptr, "READ", 4))
      {
        (*buf_ptr) += 4;
        ret.Type = TokenType_Read;
      }
      else if (substr_equal_nocase(*buf_ptr, "REINSTATE", 9))
      {
        (*buf_ptr) += 9;
        ret.Type = TokenType_Reinstate;
      }
      else if (substr_equal_nocase(*buf_ptr, "REMEMBER", 8))
      {
        (*buf_ptr) += 8;
        ret.Type = TokenType_Remember;
      }
      else if (substr_equal_nocase(*buf_ptr, "RESUME", 6))
      {
        (*buf_ptr) += 6;
        ret.Type = TokenType_Resume;
      }
      else if (substr_equal_nocase(*buf_ptr, "RETRIEVE", 8))
      {
        (*buf_ptr) += 8;
        ret.Type = TokenType_Retrieve;
      }
      break;
    case 's': case 'S':
      if (substr_equal_nocase(*buf_ptr, "STASH", 5))
      {
        (*buf_ptr) += 5;
        ret.Type = TokenType_Stash;
      }
      else if (substr_equal_nocase(*buf_ptr, "SUB", 3))
      {
        (*buf_ptr) += 3;
        ret.Type = TokenType_Sub;
      }
      break;
    case 'u': case 'U':
      if (((*buf_ptr)[1] == 'p') || ((*buf_ptr)[1] == 'P'))
      {
        (*buf_ptr) += 2;
        ret.Type = TokenType_Up;
      }
      break;
    case 'w': case 'W':
      if (substr_equal_nocase(*buf_ptr, "WRITE", 5))
      {
        (*buf_ptr) += 5;
        ret.Type = TokenType_Write;
      }
      break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      ret.Type = TokenType_Number;
      ret.Value = 0;

      while (isdigit(**buf_ptr))
      {
        int new_value = (ret.Value * 10) + (**buf_ptr - '0');

        if (new_value > 65535)
          break;

        ret.Value = new_value;
        ++*buf_ptr;
      }
      
      break;
  }

  if (ret.Type == TokenType_UnmatchedChar)
    ++*buf_ptr;

  return ret;
#undef QUO
}

static bool parse_line_tokens(StatementList *list, Token *tokens, int token_count)
{
  // TODO
  return false;
}

static void parse_line(StatementList *list, char *line, int line_length, int *row, int *column)
{
  static Token tokens_prealloc[100];
  static Token *tokens = &tokens_prealloc[0];
  static int token_offset = 0, token_count = sizeof(tokens_prealloc) / sizeof(Token);

  char *line_ptr = line;

  while (*line_ptr)
  {
    tokens[token_offset++] = pull_token(&line_ptr, row, column);

    if (token_offset == token_count)
    {
      int new_token_count = token_count * 2;
      Token *new_tokens = malloc(new_token_count * sizeof(Token));

      memcpy(new_tokens, tokens, token_count * sizeof(Token));

      free(tokens);

      tokens = new_tokens;
      token_count = new_token_count;
    }
  }

  tokens[token_offset].Type = TokenType_InvalidToken;

  if (!parse_line_tokens(list, tokens, token_offset))
  {
    ushort label = 0;
    int probability = 100;
    bool prolog_is_okay = false;
    BadStatement *bad;

    int next_token = 0;

    if ((tokens[next_token + 0].Type == TokenType_ParenLeft)
     && (tokens[next_token + 1].Type == TokenType_Number)
     && (tokens[next_token + 2].Type == TokenType_ParenRight))
    {
      label = tokens[1].Value;
      next_token += 3;
    }

    if (tokens[next_token].Type == TokenType_Please)
    {
      prolog_is_okay = true;

      next_token++;
      if (tokens[next_token].Type == TokenType_Do)
        next_token++;
    }
    else if (tokens[next_token].Type == TokenType_Do)
    {
      prolog_is_okay = true;
      next_token++;
    }

    if (prolog_is_okay)
    {
      if (tokens[next_token].Type == TokenType_Not)
      {
        next_token++;
        if ((tokens[next_token + 0].Type == TokenType_Number)
         && (tokens[next_token + 1].Type == TokenType_007))
        {
          if (tokens[next_token].Value > 100)
            complain(33, "THE IMPROBABLE IS WHAT USUALLY HAPPENS", line, *row, tokens[next_token].Column);
          probability = tokens[next_token].Value;
          next_token += 2;
        }
        probability = -probability;
      }
      else if ((tokens[next_token + 0].Type == TokenType_Number)
            && (tokens[next_token + 1].Type == TokenType_007))
      {
        if (tokens[next_token].Value > 100)
          complain(33, "THE IMPROBABLE IS WHAT USUALLY HAPPENS", line, *row, tokens[next_token].Column);
        probability = tokens[next_token].Value;
        next_token += 2;

        if (tokens[next_token].Type == TokenType_Not)
        {
          probability = -probability;
          next_token++;
        }
      }
    }

    bad = new_BadStatement(
      make_statement_header(label, probability),
      line);

    StatementList_Add(
      list,
      &bad->Statement);
  }
}

static bool found_next_statement(char *line, int *line_offset, char *line_start_token)
{
  if (*line_offset == 0)
    return false;

  if (line[*line_offset - 1] == '*')
  {
    str_copy(line_start_token, "*");
    --*line_offset;
    return true;
  }

  if (line[*line_offset - 1] == '(')
  {
    str_copy(line_start_token, "(");
    --*line_offset;
    return true;
  }

  if ((*line_offset >= 2)
   && substr_equal_nocase(line + *line_offset - 2, "DO", 2))
  {
    substr_copy(line_start_token, line + *line_offset - 2, 2);
    (*line_offset) -= 2;
    return true;
  }

  if ((*line_offset >= 6)
   && substr_equal_nocase(line + *line_offset - 6, "PLEASE", 6))
  {
    substr_copy(line_start_token, line + *line_offset - 6, 6);
    (*line_offset) -= 6;
    return true;
  }

  return false;
}

StatementList parse(FILE *input)
{
  StatementList ret = new_StatementList();
  char line_prealloc[100];
  char *line = &line_prealloc[0];
  int line_offset = 0, line_size = sizeof(line_prealloc);
  bool first_line = true;
  char line_start_token[10] = { 0 };
  int row = 1, column = 1;

  while (true)
  {
    int ch = getc(input);

    if (ch < 0)
    {
      line[line_offset] = 0;
      parse_line(&ret, line, line_offset, &row, &column);
      break;
    }

    line[line_offset++] = ch;

    if (line_offset == line_size)
    {
      int new_line_size = line_size * 2;
      char *new_line = malloc(new_line_size);

      memcpy(new_line, line, line_size);

      free(line);

      line = new_line;
      line_size = new_line_size;
    }

    if (found_next_statement(line, &line_offset, line_start_token))
    {
      line[line_offset] = 0;
      parse_line(&ret, line, line_offset, &row, &column);
      str_copy(line, line_start_token);
      line_offset = str_length(line);
    }
  }

  return ret;
}
