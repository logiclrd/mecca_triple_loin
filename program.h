#ifndef PROGRAM_H
#define PROGRAM_H

#include "types.h"

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
  ushort Index;
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
  StatementType_GiveUp,
  StatementType_WriteIn,
  StatementType_ReadOut,
  StatementType_ComeFrom,
} StatementType;

typedef struct sStatement
{
  StatementType Type;
  ushort Label;
  int Probability;
  bool Polite;
  int ErrorCode;
  int NextStatementRow; // for strict error message compatibility
  struct sStatementListNode *SuckPointDestination;
} Statement;

typedef struct sStatementListNode
{
  struct sStatementListNode *Previous, *Next;
  Statement *This;
} StatementListNode;

typedef struct sStatementList
{
  StatementListNode *First, *Last;
} StatementList;

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

  ushort Label;
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
  NumGerunds
} Gerund;

typedef struct sGerundListNode
{
  struct sGerundListNode *Previous, *Next;
  Gerund This;
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
  ExpressionList Labels;
} AbstainStatement;

typedef struct sReinstateStatement
{
  Statement Statement;

  GerundList Gerunds;
  ExpressionList Labels;
} ReinstateStatement;

typedef struct sGiveUpStatement
{
  Statement Statement;
} GiveUpStatement;

typedef struct sWriteInStatement
{
  Statement Statement;

  ExpressionList Targets;
} WriteInStatement;

typedef struct sReadOutStatement
{
  Statement Statement;

  ExpressionList Sources;
} ReadOutStatement;

typedef struct sComeFromStatement
{
  Statement Statement;

  ushort Label;
} ComeFromStatement;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ExpressionList new_ExpressionList();
void ExpressionList_Add(ExpressionList *list, Expression *item);
int ExpressionList_Count(ExpressionList *list);
void ExpressionList_Unlink(ExpressionList *list, ExpressionListNode *node);
UnaryOperator lookup_UnaryOperator(char op);
UnaryExpression *new_UnaryExpression(UnaryOperator unary_operator, Expression *inner);
BinaryOperator lookup_BinaryOperator(char op);
BinaryExpression *new_BinaryExpression(Expression *left, BinaryOperator binary_operator, Expression *right);
SubscriptExpression *new_SubscriptExpression(Expression *array, ExpressionList subscripts);
DimensionExpression *new_DimensionExpression(ExpressionList sizes);
ImmediateType lookup_ImmediateType(char type);
ImmediateExpression *new_ImmediateExpression(ImmediateType type, ushort index);
ImmediateExpression *make_onespot(ushort index);
ImmediateExpression *make_twospot(ushort index);
ImmediateExpression *make_tail(ushort index);
ImmediateExpression *make_hybrid(ushort index);
ImmediateExpression *make_mesh(ushort index);
StatementList new_StatementList();
void StatementList_Add(StatementList *list, Statement *item);
int StatementList_Count(StatementList *list);
void StatementList_Unlink(StatementList *list, StatementListNode *node);
Statement make_statement_header(int label, int probability, bool polite, int next_statement_row);
BadStatement *new_BadStatement(Statement header, char *text);
AssignmentStatement *new_AssignmentStatement(Statement header, Expression *target, Expression *value);
NextStatement *new_NextStatement(Statement header, ushort label);
ResumeStatement *new_ResumeStatement(Statement header, Expression *count);
ForgetStatement *new_ForgetStatement(Statement header, Expression *count);
StashStatement *new_StashStatement(Statement header, ExpressionList variables);
RetrieveStatement *new_RetrieveStatement(Statement header, ExpressionList variables);
IgnoreStatement *new_IgnoreStatement(Statement header, ExpressionList variables);
RememberStatement *new_RememberStatement(Statement header, ExpressionList variables);
GerundList new_GerundList();
void GerundList_Add(GerundList *list, Gerund item);
int GerundList_Count(GerundList *list);
void GerundList_Unlink(GerundList *list, GerundListNode *node);
AbstainStatement *new_AbstainStatement(Statement header, GerundList gerunds, ExpressionList labels);
ReinstateStatement *new_ReinstateStatement(Statement header, GerundList gerunds, ExpressionList labels);
GiveUpStatement *new_GiveUpStatement(Statement header);
WriteInStatement *new_WriteInStatement(Statement header, ExpressionList targets);
ReadOutStatement *new_ReadOutStatement(Statement header, ExpressionList sources);
ComeFromStatement *new_ComeFromStatement(Statement header, ushort Label);

#endif /* PROGRAM_H */

