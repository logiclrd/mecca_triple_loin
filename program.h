#ifndef PROGRAM_H
#define PROGRAM_H

#ifndef STATEMENT_DATA_TYPE
# define STATEMENT_DATA_TYPE void *
#endif /* STATEMENT_DATA_TYPE */

typedef enum eExpressionType
{
  ExpressionType_Unary,
  ExpressionType_Binary,
  ExpressionType_Subscript,
  ExpressionType_Dimension,
  ExpressionType_Immediate,
} ExpressionType;

typedef struct sExpression
{
  ExpressionType Type;
} Expression;

typedef struct sExpressionListNode
{
  struct sExpressionListNode *Previous, *Next;
  Expression *This;
} ExpressionListNode;

typedef struct sExpressionList
{
  ExpressionListNode *First, *Last;
  int Count;
} ExpressionList;

typedef enum eUnaryOperator
{
  UnaryOperator_And,
  UnaryOperator_Or,
  UnaryOperator_XOr,
} UnaryOperator;

typedef struct sUnaryExpression
{
  Expression Expression;

  UnaryOperator Operator;
  Expression *InnerExpression;
} UnaryExpression;

typedef enum eBinaryOperator
{
  BinaryOperator_Interleave,
  BinaryOperator_Select,
} BinaryOperator;

typedef struct sBinaryExpression
{
  Expression Expression;

  Expression *LeftOperand;
  BinaryOperator Operator;
  Expression *RightOperand;
} BinaryExpression;

typedef struct sSubscriptExpression
{
  Expression Expression;

  Expression *Array;
  ExpressionList Subscripts;
} SubscriptExpression;

typedef struct sDimensionExpression
{
  Expression Expression;

  ExpressionList Sizes;
} DimensionExpression;

typedef enum eImmediateType
{
  ImmediateType_OneSpot,
  ImmediateType_TwoSpot,
  ImmediateType_Tail,
  ImmediateType_Hybrid,
  ImmediateType_Mesh,
} ImmediateType;

typedef struct sImmediateExpression
{
  Expression Expression;

  ImmediateType Type;
  unsigned short Index;
} ImmediateExpression;

typedef enum eStatementType
{
  StatementType_Bad,
  StatementType_Assignment,
  StatementType_Next,
  StatementType_Resume,
  StatementType_Forget,
  StatementType_Stash,
  StatementType_Retrieve,
  StatementType_Ignore,
  StatementType_Remember,
  StatementType_Abstain,
  StatementType_Reinstate,
  StatementType_ReadIn,
  StatementType_WriteOut,
  StatementType_ComeFrom,
} StatementType;

typedef struct sStatement
{
  int Label;
  int Probability;
  StatementType Type;
} Statement;

typedef struct sBadStatement
{
  Statement Statement;

  char *Text;
} BadStatement;

typedef struct sAssignmentStatement
{
  Statement Statement;

  Expression *Target;
  Expression *Value;
} AssignmentStatement;

typedef struct sNextStatement
{
  Statement Statement;

  int Label;
} NextStatement;

typedef struct sResumeStatement
{
  Statement Statement;

  Expression *Count;
} ResumeStatement;

typedef struct sForgetStatement
{
  Statement Statement;

  Expression *Count;
} ForgetStatement;

typedef struct sStashStatement
{
  Statement Statement;

  ExpressionList Variables;
} StashStatement;

typedef struct sRetrieveStatement
{
  Statement Statement;

  ExpressionList Variables;
} RetrieveStatement;

typedef struct sIgnoreStatement
{
  Statement Statement;

  ExpressionList Variables;
} IgnoreStatement;

typedef struct sRememberStatement
{
  Statement Statement;

  ExpressionList Variables;
} RememberStatement;

typedef enum eGerund
{
  Gerund_Assigning,
  Gerund_Calculating,
  Gerund_Nexting,
  Gerund_Resuming,
  Gerund_Forgetting,
  Gerund_Stashing,
  Gerund_Retrieving,
  Gerund_Ignoring,
  Gerund_Remembering,
  Gerund_Abstaining,
  Gerund_Reinstating,
  Gerund_ReadingOut,
  Gerund_WritingIn,
  Gerund_ComingFrom,
} Gerund;

typedef struct sGerundListNode
{
  struct sGerundListNode *Previous, *Next;
  Gerund *This;
} GerundListNode;

typedef struct sGerundList
{
  GerundListNode *First, *Last;
  int Count;
} GerundList;

typedef struct sAbstainStatement
{
  Statement Statement;

  GerundList Gerunds;
} AbstainStatement;

typedef struct sReinstateStatement
{
  Statement Statement;

  GerundList Gerunds;
} ReinstateStatement;

typedef struct sReadOutStatement
{
  Statement Statement;

  Expression *Value;
} ReadOutStatement;

typedef struct sWriteInStatement
{
  Statement Statement;

  Expression *Target;
} WriteInStatement;

typedef struct sComeFromStatement
{
  Statement Statement;

  int Label;
} ComeFromStatement;

#endif /* PROGRAM_H */