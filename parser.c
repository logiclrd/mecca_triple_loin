#include <ctype.h>
#include <stdio.h>

#include "program.h"
#include "fuckup.h"
#include "types.h"

#include "syslib.i.h"

typedef enum eTokenType
{
  TokenType_InvalidToken,
  TokenType_UnmatchedChar,
  /* symbols: */
  TokenType_007,
  TokenType_And,
  TokenType_Ears,
  TokenType_Gets,
  TokenType_Hybrid,
  TokenType_Intersection,
  TokenType_Mesh,
  TokenType_Mingle,
  TokenType_OneSpot,
  TokenType_Or,
  TokenType_ParenLeft,
  TokenType_ParenRight,
  TokenType_Select,
  TokenType_Spark,
  TokenType_Splat,
  TokenType_Tail,
  TokenType_TwoSpot,
  TokenType_Wow,
  TokenType_XOr,
  /* words: */
  TokenType_Abstain,
  TokenType_Abstaining,
  TokenType_Assigning,
  TokenType_By,
  TokenType_Calculating,
  TokenType_Come,
  TokenType_Coming,
  TokenType_Do,
  TokenType_Forget,
  TokenType_Forgetting,
  TokenType_From,
  TokenType_Give,
  TokenType_Ignore,
  TokenType_Ignoring,
  TokenType_In,
  TokenType_Next,
  TokenType_Nexting,
  TokenType_Not,
  TokenType_Out,
  TokenType_Please,
  TokenType_Read,
  TokenType_Reading,
  TokenType_Reinstate,
  TokenType_Reinstating,
  TokenType_Remember,
  TokenType_Remembering,
  TokenType_Resume,
  TokenType_Resuming,
  TokenType_Retrieve,
  TokenType_Retrieving,
  TokenType_Stash,
  TokenType_Stashing,
  TokenType_Sub,
  TokenType_Up,
  TokenType_Write,
  TokenType_Writing,
  /* dynamic: */
  TokenType_Number,
} TokenType;

typedef struct sToken
{
  TokenType Type;
  int Row, Column;
  int Value;
} Token;

static Token pull_token(uchar **buf_ptr, int *row, int *column)
{
#define QUO '\''
  Token ret;

  while (isspace(**buf_ptr))
  {
    switch (**buf_ptr)
    {
      case '\b': /* backspace       */ --*column;                          break;
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
    /* symbols: */
    case '%': ++*buf_ptr; ++*column; ret.Type = TokenType_007;           break;
    case '&': ++*buf_ptr; ++*column; ret.Type = TokenType_And;           break;
    case '"': ++*buf_ptr; ++*column; ret.Type = TokenType_Ears;          break;
    case '<':
      if ((*buf_ptr)[1] == '-')
      {
        (*buf_ptr) += 2; (*column) += 2;
        ret.Type = TokenType_Gets;
      }
      break;
    case ';': ++*buf_ptr; ++*column; ret.Type = TokenType_Hybrid;        break;
    case '+': ++*buf_ptr; ++*column; ret.Type = TokenType_Intersection;  break;
    case '#': ++*buf_ptr; ++*column; ret.Type = TokenType_Mesh;          break;
    case '/':
      if (substr_equal_nocase(*buf_ptr, "/\bc", 3))
      {
        (*buf_ptr) += 3, ++*column;
        ret.Type = TokenType_Mingle;
        ret.Value = '$';
        break;
      }

      /* falls through: */
    case '$': ++*buf_ptr; ++*column; ret.Type = TokenType_Mingle;        break;
    case '.': ++*buf_ptr; ++*column; ret.Type = TokenType_OneSpot;       break;
    case 'v':
    case 'V':
      if (substr_equal_nocase(*buf_ptr, "V\b-", 3))
      {
        (*buf_ptr) += 3, ++*column;
        ret.Type = TokenType_XOr;
        ret.Value = '?';
        break;
      }

      ++*buf_ptr;
      ++*column;
      ret.Type = TokenType_Or;
      break;
    case '-':
      if (substr_equal_nocase(*buf_ptr, "-\bV", 3))
      {
        (*buf_ptr) += 3, ++*column;
        ret.Type  = TokenType_XOr;
        ret.Value = '?';
      }

      break;
    case '(': ++*buf_ptr; ++*column; ret.Type = TokenType_ParenLeft;     break;
    case ')': ++*buf_ptr; ++*column; ret.Type = TokenType_ParenRight;    break;
    case '~': ++*buf_ptr; ++*column; ret.Type = TokenType_Select;        break;
    case QUO: ++*buf_ptr; ++*column; ret.Type = TokenType_Spark;         break;
    case '*': ++*buf_ptr; ++*column; ret.Type = TokenType_Splat;         break;
    case ',': ++*buf_ptr; ++*column; ret.Type = TokenType_Tail;          break;
    case ':': ++*buf_ptr; ++*column; ret.Type = TokenType_TwoSpot;       break;
    case '!': ++*buf_ptr; ++*column; ret.Type = TokenType_Wow;           break;
    case '?': ++*buf_ptr; ++*column; ret.Type = TokenType_XOr;           break;
    /* words: */
    case 'a': case 'A':
      if (substr_equal_nocase(*buf_ptr, "ABSTAINING", 10))
      {
        (*buf_ptr) += 7; (*column) += 7;
        ret.Type = TokenType_Abstaining;
      }
      else if (substr_equal_nocase(*buf_ptr, "ABSTAIN", 7))
      {
        (*buf_ptr) += 7; (*column) += 7;
        ret.Type = TokenType_Abstain;
      }
      else if (substr_equal_nocase(*buf_ptr, "ASSIGNING", 9))
      {
        (*buf_ptr) += 9; (*column) += 9;
        ret.Type = TokenType_Assigning;
      }
      break;
    case 'b': case 'B':
      if (((*buf_ptr)[1] == 'y') || ((*buf_ptr)[1] == 'Y'))
      {
        (*buf_ptr) += 2; (*column) += 2;
        ret.Type = TokenType_By;
      }
      break;
    case 'c': case 'C':
      if (substr_equal_nocase(*buf_ptr, "c\b/", 3))
      {
        (*buf_ptr) += 3, ++*column;
        ret.Type = TokenType_Mingle;
        ret.Value = '$';
      }
      else if (substr_equal_nocase(*buf_ptr, "CALCULATING", 11))
      {
        (*buf_ptr) += 11; (*column) += 11;
        ret.Type = TokenType_Calculating;
      }
      else if (substr_equal_nocase(*buf_ptr, "COME", 4))
      {
        (*buf_ptr) += 4; (*column) += 4;
        ret.Type = TokenType_Come;
      }
      else if (substr_equal_nocase(*buf_ptr, "COMING", 6))
      {
        (*buf_ptr) += 6; (*column) += 6;
        ret.Type = TokenType_Coming;
      }
      break;
    case 'd': case 'D':
      if (((*buf_ptr)[1] == 'o') || ((*buf_ptr)[1] == 'O'))
      {
        (*buf_ptr) += 2; (*column) += 2;
        ret.Type = TokenType_Do;
      }
      break;
    case 'f': case 'F':
      if (substr_equal_nocase(*buf_ptr, "FORGETTING", 10))
      {
        (*buf_ptr) += 10; (*column) += 10;
        ret.Type = TokenType_Forgetting;
      }
      else if (substr_equal_nocase(*buf_ptr, "FORGET", 6))
      {
        (*buf_ptr) += 6; (*column) += 6;
        ret.Type = TokenType_Forget;
      }
      else if (substr_equal_nocase(*buf_ptr, "FROM", 4))
      {
        (*buf_ptr) += 4; (*column) += 4;
        ret.Type = TokenType_From;
      }
      break;
    case 'g': case 'G':
      if (substr_equal_nocase(*buf_ptr, "GIVE", 4))
      {
        (*buf_ptr) += 4; (*column) += 4;
        ret.Type = TokenType_Give;
      }
      break;
    case 'i': case 'I':
      if (substr_equal_nocase(*buf_ptr, "IGNORE", 6))
      {
        (*buf_ptr) += 6; (*column) += 6;
        ret.Type = TokenType_Ignore;
      }
      else if (substr_equal_nocase(*buf_ptr, "IGNORING", 8))
      {
        (*buf_ptr) += 8; (*column) += 8;
        ret.Type = TokenType_Ignoring;
      }
      else if (((*buf_ptr)[1] == 'n') || ((*buf_ptr)[1] == 'N'))
      {
        (*buf_ptr) += 2; (*column) += 2;
        ret.Type = TokenType_In;
      }
      break;
    case 'n': case 'N':
      if (substr_equal_nocase(*buf_ptr, "NEXTING", 7))
      {
        (*buf_ptr) += 7; (*column) += 7;
        ret.Type = TokenType_Nexting;
      }
      else if (substr_equal_nocase(*buf_ptr, "NEXT", 4))
      {
        (*buf_ptr) += 4; (*column) += 4;
        ret.Type = TokenType_Next;
      }
      else if (substr_equal_nocase(*buf_ptr, "NOT", 3)
            || substr_equal_nocase(*buf_ptr, "N'T", 3))
      {
        (*buf_ptr) += 3; (*column) += 3;
        ret.Type = TokenType_Not;
      }
      break;
    case 'o': case 'O':
      if (substr_equal_nocase(*buf_ptr, "OUT", 3))
      {
        (*buf_ptr) += 3; (*column) += 3;
        ret.Type = TokenType_Out;
      }
      break;
    case 'p': case 'P':
      if (substr_equal_nocase(*buf_ptr, "PLEASE", 6))
      {
        (*buf_ptr) += 6; (*column) += 6;
        ret.Type = TokenType_Please;
      }
      break;
    case 'r': case 'R':
      if (((*buf_ptr)[1] != 'e') && ((*buf_ptr)[1] != 'E'))
        break;
      if (substr_equal_nocase(*buf_ptr, "READING", 7))
      {
        (*buf_ptr) += 7; (*column) += 7;
        ret.Type = TokenType_Reading;
      }
      else if (substr_equal_nocase(*buf_ptr, "READ", 4))
      {
        (*buf_ptr) += 4; (*column) += 4;
        ret.Type = TokenType_Read;
      }
      else if (substr_equal_nocase(*buf_ptr, "REINSTATE", 9))
      {
        (*buf_ptr) += 9; (*column) += 9;
        ret.Type = TokenType_Reinstate;
      }
      else if (substr_equal_nocase(*buf_ptr, "REINSTATING", 11))
      {
        (*buf_ptr) += 11; (*column) += 11;
        ret.Type = TokenType_Reinstating;
      }
      else if (substr_equal_nocase(*buf_ptr, "REMEMBERING", 11))
      {
        (*buf_ptr) += 11; (*column) += 11;
        ret.Type = TokenType_Remembering;
      }
      else if (substr_equal_nocase(*buf_ptr, "REMEMBER", 8))
      {
        (*buf_ptr) += 8; (*column) += 8;
        ret.Type = TokenType_Remember;
      }
      else if (substr_equal_nocase(*buf_ptr, "RESUME", 6))
      {
        (*buf_ptr) += 6; (*column) += 6;
        ret.Type = TokenType_Resume;
      }
      else if (substr_equal_nocase(*buf_ptr, "RESUMING", 8))
      {
        (*buf_ptr) += 8; (*column) += 8;
        ret.Type = TokenType_Resuming;
      }
      else if (substr_equal_nocase(*buf_ptr, "RETRIEVE", 8))
      {
        (*buf_ptr) += 8; (*column) += 8;
        ret.Type = TokenType_Retrieve;
      }
      else if (substr_equal_nocase(*buf_ptr, "RETRIEVING", 10))
      {
        (*buf_ptr) += 10; (*column) += 10;
        ret.Type = TokenType_Retrieving;
      }
      break;
    case 's': case 'S':
      if (substr_equal_nocase(*buf_ptr, "STASHING", 8))
      {
        (*buf_ptr) += 8; (*column) += 8;
        ret.Type = TokenType_Stashing;
      }
      else if (substr_equal_nocase(*buf_ptr, "STASH", 5))
      {
        (*buf_ptr) += 5; (*column) += 5;
        ret.Type = TokenType_Stash;
      }
      else if (substr_equal_nocase(*buf_ptr, "SUB", 3))
      {
        (*buf_ptr) += 3; (*column) += 3;
        ret.Type = TokenType_Sub;
      }
      break;
    case 'u': case 'U':
      if (((*buf_ptr)[1] == 'p') || ((*buf_ptr)[1] == 'P'))
      {
        (*buf_ptr) += 2; (*column) += 2;
        ret.Type = TokenType_Up;
      }
      break;
    case 'w': case 'W':
      if (substr_equal_nocase(*buf_ptr, "WRITE", 5))
      {
        (*buf_ptr) += 5; (*column) += 5;
        ret.Type = TokenType_Write;
      }
      else if (substr_equal_nocase(*buf_ptr, "WRITING", 7))
      {
        (*buf_ptr) += 7; (*column) += 7;
        ret.Type = TokenType_Writing;
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

        do
        {
          ++*buf_ptr;

          switch (**buf_ptr)
          {
            case '\b': /* backspace       */ --*column;                          break;
            case '\t': /* horizontal tab  */ *column = 8 * ((*column + 7) >> 3); break;
            case '\n': /* newline         */ ++*row; *column = 1;                break;
            case 11:   /* vertical tab??  */                                     break;
            case 12:   /* page break      */ *row = 60 * ((*row + 59) / 60);     break;
            case '\r': /* carriage return */ *column = 1;                        break;
            case ' ':  /* space           */ ++*column;                          break;
            default:                         ++*column;                          break;
          }
        }
        while (isspace(**buf_ptr));
      }
      
      break;
  }

  if ((ret.Type == TokenType_UnmatchedChar)
   && (ret.Value != 0))
    ++*buf_ptr, ++*column;

  return ret;
#undef QUO
}

static bool is_lvalue(Expression *expression)
{
  switch (expression->Type)
  {
    case ExpressionType_Immediate:
    {
      ImmediateExpression *immediate = (ImmediateExpression *)expression;

      switch (immediate->Type)
      {
        case ImmediateType_OneSpot:
        case ImmediateType_TwoSpot:
        case ImmediateType_Tail:
        case ImmediateType_Hybrid:  return true;
      }
    }
    case ExpressionType_Subscript: return true;

    default: return false;
  }
}

static bool is_array_expression(Expression *expression)
{
  ImmediateExpression *immediate;

  if (expression->Type != ExpressionType_Immediate)
    return false;

  immediate = (ImmediateExpression *)expression;

  if (immediate->Type == ImmediateType_Tail)
    return true;
  if (immediate->Type == ImmediateType_Hybrid)
    return true;

  return false;
}

static Expression *parse_expression(Token *tokens, int token_count)
{
  int bracket_level = 0;
  static TokenType bracket_type_stack_prealloc[100];
  static TokenType *bracket_type_stack = bracket_type_stack_prealloc;
  static int bracket_type_stack_size = 100;
  bool uniform_brackets = true;
  int operator_index = -1, by_index = -1, sub_index = -1;
  ImmediateType immediate_type;
  UnaryExpression *root_unary_expression, *unary_operator_expression;
  ImmediateExpression *immediate;

  int i;

  if (token_count <= 0)
    return NULL;

  for (i=0; i < token_count; i++)
  {
    if ((tokens[i].Type == TokenType_Spark)
     || (tokens[i].Type == TokenType_Ears)
     || (tokens[i].Type == TokenType_Wow))
    {
      TokenType bracket_type = tokens[i].Type;

      if (bracket_type == TokenType_Wow)
        bracket_type = TokenType_Spark;

      if ((bracket_level > 0) && (bracket_type == bracket_type_stack[bracket_level - 1]))
        bracket_level--;
      else
      {
        if (bracket_level >= bracket_type_stack_size)
        {
          int new_bracket_type_stack_size = bracket_type_stack_size * 2;
          TokenType *new_bracket_type_stack = malloc(new_bracket_type_stack_size * sizeof(TokenType));

          memcpy(new_bracket_type_stack, bracket_type_stack, bracket_type_stack_size * sizeof(TokenType));

          if (bracket_type_stack != bracket_type_stack_prealloc)
            free(bracket_type_stack);

          bracket_type_stack = new_bracket_type_stack;
          bracket_type_stack_size = new_bracket_type_stack_size;
        }

        bracket_type_stack[bracket_level] = bracket_type;
        bracket_level++;
      }
    }

    if (bracket_level == 0)
    {
      int new_operator_index = -1;

      if (i + 1 < token_count)
        uniform_brackets = false;

      switch (tokens[i].Type)
      {
        case TokenType_Mingle:
        case TokenType_Select:
          new_operator_index = i;
          break;
        case TokenType_Sub:
          if (sub_index < 0)
            sub_index = i;
          break;
        case TokenType_By:
          if (by_index < 0)
            by_index = i;
          break;
      }

      if (new_operator_index >= 0)
      {
        if (operator_index >= 0)
          return NULL; /* the standard says an ambiguity (as operators have no "precedents") is invalid */

        operator_index = new_operator_index;
      }
    }
  }

  if (bracket_level != 0)
    return NULL; /* mismatched brackets */

  if (uniform_brackets)
  {
    int inner_expression_start = 1;
    int i;
    Expression *ret;

    while ((tokens[inner_expression_start].Type == TokenType_And)
        || (tokens[inner_expression_start].Type == TokenType_Or)
        || (tokens[inner_expression_start].Type == TokenType_XOr))
      inner_expression_start++;

    if (tokens[0].Type == TokenType_Wow)
    {
      tokens[0].Type = TokenType_OneSpot;
      tokens[0].Value = '.';

      ret = parse_expression(tokens, token_count - 1);

      tokens[0].Type = TokenType_Wow;
      tokens[0].Value = '!';

      inner_expression_start = 0;
    }
    else
      ret = parse_expression(tokens + inner_expression_start, token_count - inner_expression_start - 1);

    if (ret == NULL)
      return NULL;

    for (i=1; i < inner_expression_start; i++)
    {
      UnaryExpression *outer = new_UnaryExpression(
        lookup_UnaryOperator(tokens[i].Value),
        ret);

      if (outer == NULL)
        return NULL;

      ret = &outer->Expression;
    }

    return ret;
  }

  if ((operator_index == 0) || (operator_index == token_count - 1))
    return NULL; /* badly-formed expression */

  if (operator_index >= 0)
  {
    Expression *left = parse_expression(tokens, operator_index);
    Expression *right = parse_expression(tokens + operator_index + 1, token_count - (operator_index + 1));
    BinaryExpression *expression;

    if ((left == NULL) || (right == NULL))
      return NULL;

    expression = new_BinaryExpression(
      left,
      lookup_BinaryOperator(tokens[operator_index].Value),
      right);

    return &expression->Expression;
  }

  if (by_index >= 0)
  {
    ExpressionList dimensions = new_ExpressionList();
    DimensionExpression *expression;
    Expression *one_dimension;
    int expression_start = 0;

    while (by_index <= token_count)
    {
      one_dimension = parse_expression(&tokens[expression_start], by_index - expression_start);

      if (one_dimension == NULL)
        return NULL;

      ExpressionList_Add(&dimensions, one_dimension);

      expression_start = ++by_index;
      while ((by_index < token_count) && (tokens[by_index].Type != TokenType_By))
        by_index++;
    }

    expression = new_DimensionExpression(dimensions);

    return &expression->Expression;
  }

  if (sub_index >= 0)
  {
    SubscriptExpression *expression;
    Expression *array;
    ExpressionList subscripts = new_ExpressionList();

    int expression_start;

    array = parse_expression(tokens, sub_index);

    if ((array == NULL)
     || !is_array_expression(array))
      return NULL;

    expression_start = sub_index + 1;

    while (expression_start < token_count)
    {
      int i;
      Expression *one_subscript;

      for (i = token_count; i > expression_start; i--)
      {
        one_subscript = parse_expression(&tokens[expression_start], i - expression_start);

        if (one_subscript != NULL)
          break;
      }

      if (one_subscript == NULL)
        return NULL;

      ExpressionList_Add(&subscripts, one_subscript);

      expression_start = i;
    }

    expression = new_SubscriptExpression(array, subscripts);

    return &expression->Expression;
  }

  switch (tokens[0].Type)
  {
    case TokenType_OneSpot: immediate_type = ImmediateType_OneSpot; break;
    case TokenType_TwoSpot: immediate_type = ImmediateType_TwoSpot; break;
    case TokenType_Tail:    immediate_type = ImmediateType_Tail;    break;
    case TokenType_Hybrid:  immediate_type = ImmediateType_Hybrid;  break;
    case TokenType_Mesh:    immediate_type = ImmediateType_Mesh;    break;

    default:
      return NULL; /* unrecognized token */
  }

  tokens++, token_count--;

  unary_operator_expression = root_unary_expression = NULL;

  while (true)
  {
    switch (tokens[0].Type)
    {
      case TokenType_And:
      case TokenType_Or:
      case TokenType_XOr:
      {
        UnaryOperator unary_operator = lookup_UnaryOperator(tokens[0].Value);

        UnaryExpression *inner = new_UnaryExpression(unary_operator, NULL);

        if (unary_operator_expression != NULL)
          unary_operator_expression->InnerExpression = &inner->Expression;
        else
          root_unary_expression = inner;

        unary_operator_expression = inner;

        tokens++, token_count--;

        continue;
      }
    }

    break;
  }

  if ((token_count != 1)
   || (tokens[0].Type != TokenType_Number))
    return NULL;

  immediate = new_ImmediateExpression(immediate_type, tokens[0].Value);

  if (unary_operator_expression == NULL)
    return &immediate->Expression;
  else
  {
    unary_operator_expression->InnerExpression = &immediate->Expression;

    return &root_unary_expression->Expression;
  }
}

static bool parse_variable_list(Token *token, int token_count, ExpressionList *variables)
{
  *variables = new_ExpressionList();

  while (token_count > 0)
  {
    ImmediateType type;
    ImmediateExpression *variable;

    switch (token->Type)
    {
      case TokenType_OneSpot: type = ImmediateType_OneSpot; break;
      case TokenType_TwoSpot: type = ImmediateType_TwoSpot; break;
      case TokenType_Tail:    type = ImmediateType_Tail;    break;
      case TokenType_Hybrid:  type = ImmediateType_Hybrid;  break;

      default: return false;
    }

    token++, token_count--;

    if (token->Type != TokenType_Number)
      return false;

    variable = new_ImmediateExpression(type, token->Value);

    ExpressionList_Add(variables, &variable->Expression);

    token++, token_count--;

    if ((token_count != 0) && (token->Type != TokenType_Intersection))
      return false;

    token++, token_count--;
  }

  return true;
}

static bool parse_gerund_and_label_lists(Token *token, int token_count, GerundList *gerunds, ExpressionList *labels)
{
  *gerunds = new_GerundList();
  *labels = new_ExpressionList();

  while (token_count > 0)
  {
    Gerund gerund;

    switch (token->Type)
    {
      case TokenType_Assigning:   gerund = Gerund_Assigning;    break;
      case TokenType_Calculating: gerund = Gerund_Calculating;  break;
      case TokenType_Nexting:     gerund = Gerund_Nexting;      break;
      case TokenType_Resuming:    gerund = Gerund_Resuming;     break;
      case TokenType_Forgetting:  gerund = Gerund_Forgetting;   break;
      case TokenType_Stashing:    gerund = Gerund_Stashing;     break;
      case TokenType_Retrieving:  gerund = Gerund_Retrieving;   break;
      case TokenType_Ignoring:    gerund = Gerund_Ignoring;     break;
      case TokenType_Remembering: gerund = Gerund_Remembering;  break;
      case TokenType_Abstaining:  gerund = Gerund_Abstaining;   break;
      case TokenType_Reinstating: gerund = Gerund_Reinstating;  break;

      case TokenType_Reading: 
      case TokenType_Writing: 
      case TokenType_Coming:
      {
        if (token_count <= 1)
          return false;

        switch (token[0].Type)
        {
          case TokenType_Reading:
            if (token[1].Type != TokenType_Out)
              return false;
            gerund = Gerund_ReadingOut;
            break;
          case TokenType_Writing:
            if (token[1].Type != TokenType_In)
              return false;
            gerund = Gerund_WritingIn;
            break;
          case TokenType_Coming:
            if (token[1].Type != TokenType_From)
              return false;
            gerund = Gerund_ComingFrom;
            break;

          default: return false;
        }

        token++, token_count--;

        break;
      }

      case TokenType_ParenLeft:
      {
        token++, token_count--;

        if ((token[0].Type == TokenType_Number)
         && (token[1].Type == TokenType_ParenRight))
        {
          ushort label = token[0].Value;
          ImmediateExpression *expression;

          if (label == 0)
            return false;

          expression = new_ImmediateExpression(ImmediateType_Mesh, label);

          ExpressionList_Add(labels, &expression->Expression);

          token += 2, token_count -= 2;

          if ((token_count != 0) && (token->Type != TokenType_Intersection))
            return false;

          continue;
        }
        else
          return false;
      }

      default: return false;
    }

    GerundList_Add(gerunds, gerund);

    token++, token_count--;

    if ((token_count != 0) && (token->Type != TokenType_Intersection))
      return false;

    token++, token_count--;
  }

  return true;
}

static bool parse_expression_list(Token *token, int token_count, ExpressionList *expressions)
{
  *expressions = new_ExpressionList();

  while (token_count > 0)
  {
    int intersection;
    Expression *expression;

    intersection = 0;
    while ((intersection < token_count)
        && (token[intersection].Type != TokenType_Intersection))
      intersection++;

    if (intersection == token_count - 1)
      return false;

    expression = parse_expression(token, intersection);

    if (expression == NULL)
      return false;

    ExpressionList_Add(expressions, expression);

    intersection++;

    token += intersection, token_count -= intersection;
  }

  return true;
}

static bool parse_line_tokens(StatementList *list, Token *tokens, int token_count, int next_statement_row)
{
  ushort label = 0;
  int probability = 100;
  bool polite = false;

  Statement header;
  Statement *item = NULL;

  int next_token = 0;

  if (tokens[next_token].Type == TokenType_Splat)
    return false;

  if ((tokens[next_token + 0].Type == TokenType_ParenLeft)
   && (tokens[next_token + 1].Type == TokenType_Number)
   && (tokens[next_token + 2].Type == TokenType_ParenRight))
  {
    label = tokens[1].Value;

    if (label == 0)
      return false;

    next_token += 3;
  }

  if (tokens[next_token].Type == TokenType_Please)
  {
    polite = true;

    next_token++;
    if (tokens[next_token].Type == TokenType_Do)
      next_token++;
  }
  else if (tokens[next_token].Type == TokenType_Do)
    next_token++;
  else
    return false; /* prolog is not okay */

  if (tokens[next_token].Type == TokenType_Not)
  {
    next_token++;
    if ((tokens[next_token + 0].Type == TokenType_007)
     && (tokens[next_token + 1].Type == TokenType_Number))
    {
      if (tokens[next_token + 1].Value > 100)
        header.ErrorCode = 33;
      probability = tokens[next_token + 1].Value;
      next_token += 2;
    }
    probability = -probability;
  }
  else if ((tokens[next_token + 0].Type == TokenType_007)
        && (tokens[next_token + 1].Type == TokenType_Number))
  {
    if (tokens[next_token + 1].Value > 100)
      header.ErrorCode = 33;
    probability = tokens[next_token + 1].Value;
    next_token += 2;

    if (tokens[next_token].Type == TokenType_Not)
    {
      probability = -probability;
      next_token++;
    }
  }

  header = make_statement_header(label, probability, polite, next_statement_row);

  /*

  Types of statements:

    expr <- expr
    (label) NEXT
    RESUME expr
    FORGET expr
    STASH list
    RETRIEVE list
    IGNORE list
    REMEMBER list
    ABSTAIN FROM list
    REINSTATE list
    READ OUT expr
    WRITE IN expr
    COME FROM (label)

  Only one of these is not immediately identifiable based on the initial
  token, so if the initial token isn't one of the other ones, we assume
  that it is an assignment expression and try to decode it as one.

  */

  switch (tokens[next_token].Type)
  {
    case TokenType_ParenLeft: /* (label) NEXT */
    {
      NextStatement *statement;

      if ((tokens[next_token + 1].Type != TokenType_Number)
       || (tokens[next_token + 2].Type != TokenType_ParenRight)
       || (tokens[next_token + 3].Type != TokenType_Next)
       || (next_token + 4 < token_count))
        return false;

      if (tokens[next_token + 1].Value == 0)
        return false;

      statement = new_NextStatement(header, tokens[next_token + 1].Value);
      item = &statement->Statement;

      break;
    }
    case TokenType_Resume: /* RESUME expr */
    {
      ResumeStatement *statement;
      
      Expression *expression = parse_expression(&tokens[next_token + 1], token_count - (next_token + 1));

      if (expression == NULL)
        return false;

      statement = new_ResumeStatement(header, expression);
      item = &statement->Statement;

      break;
    }
    case TokenType_Forget: /* FORGET expr */
    {
      ForgetStatement *statement;
      
      Expression *expression = parse_expression(&tokens[next_token + 1], token_count - (next_token + 1));

      if (expression == NULL)
        return false;

      statement = new_ForgetStatement(header, expression);
      item = &statement->Statement;

      break;
    }
    case TokenType_Stash: /* STASH list */
    {
      StashStatement *statement;
      ExpressionList variables;

      if (!parse_variable_list(&tokens[next_token + 1], token_count - (next_token + 1), &variables))
        return false;

      statement = new_StashStatement(header, variables);
      item = &statement->Statement;

      break;
    }
    case TokenType_Retrieve: /* RETRIEVE list */
    {
      RetrieveStatement *statement;
      ExpressionList variables;

      if (!parse_variable_list(&tokens[next_token + 1], token_count - (next_token + 1), &variables))
        return false;

      statement = new_RetrieveStatement(header, variables);
      item = &statement->Statement;

      break;
    }
    case TokenType_Ignore: /* IGNORE list */
    {
      IgnoreStatement *statement;
      ExpressionList variables;

      if (!parse_variable_list(&tokens[next_token + 1], token_count - (next_token + 1), &variables))
        return false;

      statement = new_IgnoreStatement(header, variables);
      item = &statement->Statement;

      break;
    }
    case TokenType_Remember: /* REMEMBER list */
    {
      RememberStatement *statement;
      ExpressionList variables;

      if (!parse_variable_list(&tokens[next_token + 1], token_count - (next_token + 1), &variables))
        return false;

      statement = new_RememberStatement(header, variables);
      item = &statement->Statement;

      break;
    }
    case TokenType_Abstain: /* ABSTAIN FROM list */
    {
      AbstainStatement *statement;
      GerundList gerunds;
      ExpressionList labels;

      if ((tokens[next_token + 1].Type != TokenType_From)
       || !parse_gerund_and_label_lists(&tokens[next_token + 2], token_count - (next_token + 2), &gerunds, &labels))
        return false;

      statement = new_AbstainStatement(header, gerunds, labels);
      item = &statement->Statement;

      break;
    }
    case TokenType_Reinstate: /* REINSTATE list */
    {
      ReinstateStatement *statement;
      GerundList gerunds;
      ExpressionList labels;

      if (!parse_gerund_and_label_lists(&tokens[next_token + 1], token_count - (next_token + 1), &gerunds, &labels))
        return false;

      statement = new_ReinstateStatement(header, gerunds, labels);
      item = &statement->Statement;

      break;
    }
    case TokenType_Give: /* GIVE UP */
    {
      GiveUpStatement *statement;

      if ((tokens[next_token + 1].Type != TokenType_Up)
       || (next_token + 2 < token_count))
        return false;

      statement = new_GiveUpStatement(header);
      item = &statement->Statement;

      break;
    }
    case TokenType_Write: /* WRITE IN expr+expr+expr */
    {
      WriteInStatement *statement;
      ExpressionList targets;
      ExpressionListNode *trace;
      
      if ((tokens[next_token + 1].Type != TokenType_In)
       || !parse_expression_list(&tokens[next_token + 2], token_count - (next_token + 2), &targets))
        return false;

      trace = targets.First;
      while (trace != NULL)
      {
        if (!is_lvalue(trace->This))
          return false;

        trace = trace->Next;
      }

      statement = new_WriteInStatement(header, targets);
      item = &statement->Statement;

      break;
    }
    case TokenType_Read: /* READ OUT expr+expr+expr */
    {
      ReadOutStatement *statement;
      ExpressionList sources;
      
      if ((tokens[next_token + 1].Type != TokenType_Out)
       || !parse_expression_list(&tokens[next_token + 2], token_count - (next_token + 2), &sources))
        return false;

      statement = new_ReadOutStatement(header, sources);
      item = &statement->Statement;

      break;
    }
    case TokenType_Come: /* COME FROM (label) */
    {
      ComeFromStatement *statement;

      if ((tokens[next_token + 1].Type != TokenType_From)
       || (tokens[next_token + 2].Type != TokenType_ParenLeft)
       || (tokens[next_token + 3].Type != TokenType_Number)
       || (tokens[next_token + 4].Type != TokenType_ParenRight)
       || (next_token + 5 < token_count))
        return false;

      if (tokens[next_token + 3].Value == 0)
        return false;

      statement = new_ComeFromStatement(header, tokens[next_token + 3].Value);
      item = &statement->Statement;

      break;
    }
    default: /* try for: expr <- expr */
    {
      AssignmentStatement *statement;
      int gets_index;
      Expression *target, *value;

      for (gets_index = next_token; gets_index < token_count; gets_index++)
        if (tokens[gets_index].Type == TokenType_Gets)
          break;

      if (gets_index >= token_count)
        return false;

      target = parse_expression(&tokens[next_token], gets_index - next_token);
      value = parse_expression(&tokens[gets_index + 1], token_count - (gets_index + 1));

      if ((target == NULL)
       || (value == NULL)
       || !is_lvalue(target))
        return false;

      statement = new_AssignmentStatement(header, target, value);
      item = &statement->Statement;

      break;
    }
  }

  if (item != NULL)
  {
    StatementList_Add(list, item);
    return true;
  }
  else
    return false;
}

static void parse_line(StatementList *list, uchar *line, int line_length, int *row, int *column)
{
  static Token tokens_prealloc[100];
  static Token *tokens = &tokens_prealloc[0];
  static int token_count = sizeof(tokens_prealloc) / sizeof(Token);
  int token_offset = 0;

  uchar *line_ptr = line;

  while (*line_ptr)
  {
    tokens[token_offset++] = pull_token(&line_ptr, row, column);

    if ((tokens[token_offset - 1].Type == TokenType_UnmatchedChar)
     && (tokens[token_offset - 1].Value == 0))
    {
      token_offset--;
      break;
    }

    if (token_offset == token_count)
    {
      int new_token_count = token_count * 2;
      Token *new_tokens = malloc(new_token_count * sizeof(Token));

      memcpy(new_tokens, tokens, token_count * sizeof(Token));

      if (tokens != tokens_prealloc)
        free(tokens);

      tokens = new_tokens;
      token_count = new_token_count;
    }
  }

  if (token_offset == 0)
    return; /* it is not shameful to have empty statements */

  tokens[token_offset].Type = TokenType_InvalidToken;

  if (!parse_line_tokens(list, tokens, token_offset, *row))
  {
    ushort label = 0;
    int probability = 100;
    bool prolog_is_okay = false;
    bool polite = false;
    BadStatement *bad;

    int next_token = 0;

    if ((tokens[next_token + 0].Type == TokenType_ParenLeft)
     && (tokens[next_token + 1].Type == TokenType_Number)
     && (tokens[next_token + 2].Type == TokenType_ParenRight))
    {
      label = tokens[1].Value;

      if (label == 0)
        complain(197, error_code_to_string(197), NULL, 0, 0);

      next_token += 3;
    }

    if (tokens[next_token].Type == TokenType_Please)
    {
      prolog_is_okay = true;
      polite = true;

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
      make_statement_header(label, probability, polite, *row),
      strdup(line));

    StatementList_Add(
      list,
      &bad->Statement);
  }
}

static bool back_trace_accepts_label(uchar *line, int length)
{
  int offset = length - 1;

  while (isspace(line[offset]))
    offset--;

  if (line[offset] == '+')
    return true;

  if ((offset >= 1)
   && (toupper(line[offset - 1]) == 'D')
   && (toupper(line[offset - 0]) == 'O'))
    return true;

  if ((offset >= 2)
   && (toupper(line[offset - 2]) == 'N')
   && ((toupper(line[offset - 1]) == 'O') || (line[offset - 1] == '\''))
   && (toupper(line[offset - 0]) == 'T'))
    return back_trace_accepts_label(line, offset - 2);

  if ((offset >= 5)
   && (toupper(line[offset - 5]) == 'P')
   && (toupper(line[offset - 4]) == 'L')
   && (toupper(line[offset - 3]) == 'E')
   && (toupper(line[offset - 2]) == 'A')
   && (toupper(line[offset - 1]) == 'S')
   && (toupper(line[offset - 0]) == 'E'))
    return true;

  if ((offset >= 3)
   && (toupper(line[offset - 3]) == 'F')
   && (toupper(line[offset - 2]) == 'R')
   && (toupper(line[offset - 1]) == 'O')
   && (toupper(line[offset - 0]) == 'M'))
  {
    /* need to see farther back: could be
     * accepted:                    DO COME FROM (5) ...
     *                           DO ABSTAIN FROM (3) ...
     * not accepted: DO ABSTAIN FROM COMING FROM ... ...
     */
    offset -= 4;
    while ((offset >= 0) && isspace(line[offset]))
      offset--;

    if ((offset >= 3)
     && (toupper(line[offset - 3]) == 'C')
     && (toupper(line[offset - 2]) == 'O')
     && (toupper(line[offset - 1]) == 'M')
     && (toupper(line[offset - 0]) == 'E'))
      return true;

    if ((offset >= 6)
     && (toupper(line[offset - 6]) == 'A')
     && (toupper(line[offset - 5]) == 'B')
     && (toupper(line[offset - 4]) == 'S')
     && (toupper(line[offset - 3]) == 'T')
     && (toupper(line[offset - 2]) == 'A')
     && (toupper(line[offset - 1]) == 'I')
     && (toupper(line[offset - 0]) == 'N'))
      return true;

    return false;
  }

  if ((offset >= 8)
   && (toupper(line[offset - 8]) == 'R')
   && (toupper(line[offset - 7]) == 'E')
   && (toupper(line[offset - 6]) == 'I')
   && (toupper(line[offset - 5]) == 'N')
   && (toupper(line[offset - 4]) == 'S')
   && (toupper(line[offset - 3]) == 'T')
   && (toupper(line[offset - 2]) == 'A')
   && (toupper(line[offset - 1]) == 'T')
   && (toupper(line[offset - 0]) == 'E'))
    return true;

  return false;
}

typedef struct sStatementFinderState
{
  bool previous_word_is_please;
  bool clear_previous_word_is_please;
  bool have_statement_identifier;
  bool parsing_come_from;
} StatementFinderState;

static bool found_next_statement(uchar *line, int *line_offset, uchar *line_start_token, StatementFinderState *state)
{
  bool previous_word_is_please = state->previous_word_is_please;

  if (*line_offset == 0)
    return false;

  if (isspace(line[*line_offset - 1]))
    return false;

  if ((state->clear_previous_word_is_please == false)
   && (toupper(line[*line_offset - 1]) == 'D'))
    state->clear_previous_word_is_please = true;
  else
    state->previous_word_is_please = state->clear_previous_word_is_please = false;

  if (line[*line_offset - 1] == '*')
  {
    str_copy(line_start_token, "*");
    --*line_offset;
    return true;
  }

  if (line[*line_offset - 1] == '(')
  {
    if (state->parsing_come_from || back_trace_accepts_label(line, *line_offset - 1))
    {
      state->parsing_come_from = false;
      return false;
    }
    else
    {
      str_copy(line_start_token, "(");
      --*line_offset;
      return true;
    }
  }

  if ((*line_offset >= 2)
   && substr_equal_nocase(line + *line_offset - 2, "DO", 2))
  {
    if (previous_word_is_please)
      return false;

    if (state->have_statement_identifier)
    {
      substr_copy(line_start_token, line + *line_offset - 2, 2);
      (*line_offset) -= 2;
      return true;
    }

    state->have_statement_identifier = true;
  }

  if ((*line_offset >= 4)
   && substr_equal_nocase(line + *line_offset - 4, "COME", 4))
  {
    state->parsing_come_from = true;
    return false;
  }

  if ((*line_offset >= 6)
   && substr_equal_nocase(line + *line_offset - 6, "PLEASE", 6))
  {
    if (state->have_statement_identifier)
    {
      substr_copy(line_start_token, line + *line_offset - 6, 6);
      (*line_offset) -= 6;
      return true;
    }

    state->have_statement_identifier = true;
    state->previous_word_is_please = true;
  }

  return false;
}

static void start_next_statement(uchar *line, int *line_offset, uchar *line_start_token, StatementFinderState *state)
{
  str_copy(line, line_start_token);
  *line_offset = str_length(line);
  line_start_token[0] = 0;

  memset(state, 0, sizeof(*state));

  state->previous_word_is_please = substr_equal_nocase(line, "PLEASE", 6);
  state->have_statement_identifier = state->previous_word_is_please || substr_equal_nocase(line, "DO", 2);
}

typedef struct sInput
{
  FILE *file;
  const char *string;
} Input;

int read_char(Input *input)
{
  if (input->file != NULL)
  {
    int ret = fgetc(input->file);

    if (ret < 0)
      input->file = NULL;

    return ret;
  }

  if (input->string != NULL)
  {
    int ret = *input->string;

    if (ret == 0)
      ret = -1, input->string = NULL;
    else
      ++input->string;

    return ret;
  }

  return -1;
}

static void parse_to_list(Input *input, StatementList *ret)
{
  uchar line_prealloc[100];
  uchar *line = &line_prealloc[0];
  int line_offset = 0, line_size = sizeof(line_prealloc);
  bool first_line = true;
  uchar line_start_token[10] = { 0 };
  int row = 1, column = 1;

  StatementFinderState state = { 0 };

  while (true)
  {
    int ch = read_char(input);

    if (ch < 0)
    {
      line[line_offset] = 0;
      parse_line(ret, line, line_offset, &row, &column);
      break;
    }

    line[line_offset++] = ch;

    if (line_offset == line_size)
    {
      int new_line_size = line_size * 2;
      uchar *new_line = malloc(new_line_size);

      memcpy(new_line, line, line_size);

      if (line != line_prealloc)
        free(line);

      line = new_line;
      line_size = new_line_size;
    }

    if (found_next_statement(line, &line_offset, line_start_token, &state))
    {
      line[line_offset] = 0;
      parse_line(ret, line, line_offset, &row, &column);

      start_next_statement(line, &line_offset, line_start_token, &state);
    }
  }
}

static void check_politeness_level(StatementList program)
{
  int total_count, polite_count;
  int ratio;

  StatementListNode *trace = program.First;

  total_count = polite_count = 0;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    total_count++;
    if (statement->Polite)
      polite_count++;

    trace = trace->Next;
  }

  if (total_count < 3)
    return;

  ratio = (polite_count != 0) ? (total_count - 1) / polite_count : 10;

  if (ratio > 5) /* less than 1/5 are polite */
    complain(79, error_code_to_string(79), NULL, 0, 0);

  ratio = total_count / polite_count;

  if (ratio < 3) /* more than 1/3 are polite */
    complain(99, error_code_to_string(99), NULL, 0, 0);
}

static bool calls_system_library(StatementList program)
{
  StatementListNode *trace;
  bool have_use = false;

  bool user_defined[2000];

  memset(user_defined, 0, sizeof(user_defined));

  trace = program.First;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    if ((statement->Label >= 1000) && (statement->Label < 2000))
      user_defined[statement->Label] = true;

    trace = trace->Next;
  }

  trace = program.First;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    if (statement->Type == StatementType_Next)
    {
      NextStatement *next = (NextStatement *)statement;

      switch (next->Label)
      {
        case 1000: /* .3 <- .1 + .2 */
        case 1009: /* .3 <- unchecked(.1 + .2), .4 <- (overflow == false) ? #1 : #2 */
        case 1010: /* .3 <- .1 - .2 */
        case 1020: /* .1++ */
        case 1030: /* .3 <- .1 * .2 */
        case 1039: /* .3 <- unchecked(.1 * .2), .4 <- (overflow == false) ? #1 : #2 */
        case 1040: /* .3 <- (.2 != 0) ? .1 / .2 : #0 */
        case 1050: /* .2 <- (.1 != 0) ? :1 / .1 : #0 */
        case 1060: /* .3 <- .1 | .2 */
        case 1070: /* .3 <- .1 & .2 */
        case 1080: /* .3 <- .1 ^ .2 */
        case 1500: /* :3 <- :1 + :2 */
        case 1509: /* :3 <- unchecked(:1 + :2), :4 <- (overflow == false) ? #1 : #2 */
        case 1510: /* :3 <- :1 - :2 */
        case 1520: /* :1 <- (.1 << 16) | .2 */
        case 1525: /* ("undocumented") .3 <<= 8 */
        case 1530: /* :1 <- .1 * .2 */
        case 1540: /* :3 <- :1 * :2 */
        case 1549: /* :3 <- unchecked(:1 * :2), .4 <- (overflow == false) ? #1 : #2 */
        case 1550: /* :3 <- (:2 != 0) ? :1 / :2 : #0 */
        case 1900: /* .1 <- uniform random no. from #0 to #65535 */
        case 1910: /* .2 <- normal random no. from #0 to .1, with standard deviation .1 divided by #12 */
          if (user_defined[next->Label])
            return false;

          have_use = true;
      }
    }

    trace = trace->Next;
  }

  return have_use;
}

StatementList parse(FILE *file)
{
  Input input = { file };
  Input system_library = { 0, IntercalSystemLibrary };

  StatementList ret = new_StatementList();

  parse_to_list(&input, &ret);

  StatementList_Add(
    &ret,
    (Statement *)new_EndOfCodeModuleStatement(
      make_statement_header(0, 100, false, -1),
      "user"));

  check_politeness_level(ret);

  if (calls_system_library(ret))
  {
    parse_to_list(&system_library, &ret);

    StatementList_Add(
      &ret,
      (Statement *)new_EndOfCodeModuleStatement(
        make_statement_header(0, 100, false, -1),
        "syslib"));
  }

  return ret;
}

