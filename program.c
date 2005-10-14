#include "program.h"
#include "fuckup.h"
#include "types.h"

ExpressionList new_ExpressionList()
{
  ExpressionList ret;

  ret.First = ret.Last = NULL;

  return ret;
}

void ExpressionList_Add(ExpressionList *list, Expression *item)
{
  ExpressionListNode *new_node = alloc(ExpressionListNode);

  new_node->Previous = list->Last;
  new_node->Next = NULL;
  new_node->This = item;

  if (list->Last)
    list->Last->Next = new_node;

  if (!list->First)
    list->First = new_node;
  list->Last = new_node;
}

int ExpressionList_Count(ExpressionList *list)
{
  ExpressionListNode *trace = list->First;
  int count = (trace != NULL);

  while (trace != list->Last)
    count++, trace = trace->Next;

  return count;
}

void ExpressionList_Unlink(ExpressionList *list, ExpressionListNode *node)
{
  if (list->First == node)
    list->First = list->First->Next;
  if (list->Last == node)
    list->Last = list->Last->Previous;

  if (node->Previous && (node->Previous->Next == node))
    node->Previous->Next = node->Next;
  if (node->Next && (node->Next->Previous == node))
    node->Next->Previous = node->Previous;

  node->Previous = node->Next = NULL;

  free(node);
}

UnaryOperator lookup_UnaryOperator(char op)
{
  switch (op)
  {
    case '&': return UnaryOperator_And;
    case 'V': return UnaryOperator_Or;
    case '?': return UnaryOperator_XOr;

    default:
      explode("BAD LUCK IN MODULE P-U-O");
  }
}

UnaryExpression *new_UnaryExpression(UnaryOperator unary_operator, Expression *inner)
{
  UnaryExpression *ret = alloc(UnaryExpression);

  ret->Expression.Type = ExpressionType_Unary;

  ret->Operator = unary_operator;
  ret->InnerExpression = inner;

  return ret;
}

BinaryOperator lookup_BinaryOperator(char op)
{
  switch (op)
  {
    case '$':
    case '/': return BinaryOperator_Interleave;
    case '~': return BinaryOperator_Select;

    default:
      explode("BAD LUCK IN MODULE P-B-O");
  }
}

BinaryExpression *new_BinaryExpression(Expression *left, BinaryOperator binary_operator, Expression *right)
{
  BinaryExpression *ret = alloc(BinaryExpression);

  ret->Expression.Type = ExpressionType_Binary;

  ret->LeftOperand = left;
  ret->Operator = binary_operator;
  ret->RightOperand = right;

  return ret;
}

SubscriptExpression *new_SubscriptExpression(Expression *array, ExpressionList subscripts)
{
  SubscriptExpression *ret = alloc(SubscriptExpression);

  ret->Expression.Type = ExpressionType_Subscript;

  ret->Array = array;
  ret->Subscripts = subscripts;

  return ret;
}

DimensionExpression *new_DimensionExpression(ExpressionList sizes)
{
  DimensionExpression *ret = alloc(DimensionExpression);

  ret->Expression.Type = ExpressionType_Dimension;

  ret->Sizes = sizes;

  return ret;
}

ImmediateType lookup_ImmediateType(char type)
{
  switch (type)
  {
    case '.': return ImmediateType_OneSpot;
    case ':': return ImmediateType_TwoSpot;
    case ',': return ImmediateType_Tail;
    case ';': return ImmediateType_Hybrid;
    case '#': return ImmediateType_Mesh;

    default:
      explode("BAD LUCK IN MODULE P-I-T");
  }
}

ImmediateExpression *new_ImmediateExpression(ImmediateType type, ushort index)
{
  ImmediateExpression *ret = alloc(ImmediateExpression);

  if ((index == 0) && (type != ImmediateType_Mesh))
    explode("BAD LUCK IN MODULE P-N-I-E");

  ret->Expression.Type = ExpressionType_Immediate;

  ret->Type = type;
  ret->Index = index;

  return ret;
}

ImmediateExpression *make_onespot(ushort index)
{
  return new_ImmediateExpression(ImmediateType_OneSpot, index);
}

ImmediateExpression *make_twospot(ushort index)
{
  return new_ImmediateExpression(ImmediateType_TwoSpot, index);
}

ImmediateExpression *make_tail(ushort index)
{
  return new_ImmediateExpression(ImmediateType_Tail, index);
}

ImmediateExpression *make_hybrid(ushort index)
{
  return new_ImmediateExpression(ImmediateType_Hybrid, index);
}

ImmediateExpression *make_mesh(ushort index)
{
  return new_ImmediateExpression(ImmediateType_Mesh, index);
}

StatementList new_StatementList()
{
  StatementList ret;

  ret.First = ret.Last = NULL;

  return ret;
}

void StatementList_Add(StatementList *list, Statement *item)
{
  StatementListNode *new_node = alloc(StatementListNode);

  new_node->Previous = list->Last;
  new_node->Next = NULL;
  new_node->This = item;

  if (list->Last)
    list->Last->Next = new_node;

  if (!list->First)
    list->First = new_node;
  list->Last = new_node;
}

int StatementList_Count(StatementList *list)
{
  StatementListNode *trace = list->First;
  int count = (trace != NULL);

  while (trace != list->Last)
    count++, trace = trace->Next;

  return count;
}

void StatementList_Unlink(StatementList *list, StatementListNode *node)
{
  if (list->First == node)
    list->First = list->First->Next;
  if (list->Last == node)
    list->Last = list->Last->Previous;

  if (node->Previous && (node->Previous->Next == node))
    node->Previous->Next = node->Next;
  if (node->Next && (node->Next->Previous == node))
    node->Next->Previous = node->Previous;

  node->Previous = node->Next = NULL;

  free(node);
}

Statement make_statement_header(int label, int probability, bool polite, int next_statement_row)
{
  Statement ret = { 0 };

  ret.Label = label;
  ret.Probability = probability;
  ret.Polite = polite;
  ret.NextStatementRow = next_statement_row;

  return ret;
}

BadStatement *new_BadStatement(Statement header, char *text)
{
  BadStatement *ret = alloc(BadStatement);

  header.Type = StatementType_Bad;
  ret->Statement = header;

  ret->Text = text;

  return ret;
}

EndOfCodeModuleStatement *new_EndOfCodeModuleStatement(Statement header, char *module_name)
{
  EndOfCodeModuleStatement *ret = alloc(EndOfCodeModuleStatement);

  header.Type = StatementType_EndOfCodeModule;
  ret->Statement = header;

  ret->ModuleName = module_name;

  return ret;
}

AssignmentStatement *new_AssignmentStatement(Statement header, Expression *target, Expression *value)
{
  AssignmentStatement *ret = alloc(AssignmentStatement);

  header.Type = StatementType_Assignment;
  ret->Statement = header;

  ret->Target = target;
  ret->Value = value;

  return ret;
}

NextStatement *new_NextStatement(Statement header, ushort label)
{
  NextStatement *ret = alloc(NextStatement);

  header.Type = StatementType_Next;
  ret->Statement = header;

  ret->Label = label;

  return ret;
}

ResumeStatement *new_ResumeStatement(Statement header, Expression *count)
{
  ResumeStatement *ret = alloc(ResumeStatement);

  header.Type = StatementType_Resume;
  ret->Statement = header;

  ret->Count = count;

  return ret;
}

ForgetStatement *new_ForgetStatement(Statement header, Expression *count)
{
  ForgetStatement *ret = alloc(ForgetStatement);

  header.Type = StatementType_Forget;
  ret->Statement = header;

  ret->Count = count;

  return ret;
}

StashStatement *new_StashStatement(Statement header, ExpressionList variables)
{
  StashStatement *ret = alloc(StashStatement);

  header.Type = StatementType_Stash;
  ret->Statement = header;

  ret->Variables = variables;

  return ret;
}

RetrieveStatement *new_RetrieveStatement(Statement header, ExpressionList variables)
{
  RetrieveStatement *ret = alloc(RetrieveStatement);

  header.Type = StatementType_Retrieve;
  ret->Statement = header;

  ret->Variables = variables;

  return ret;
}

IgnoreStatement *new_IgnoreStatement(Statement header, ExpressionList variables)
{
  IgnoreStatement *ret = alloc(IgnoreStatement);

  header.Type = StatementType_Ignore;
  ret->Statement = header;

  ret->Variables = variables;

  return ret;
}

RememberStatement *new_RememberStatement(Statement header, ExpressionList variables)
{
  RememberStatement *ret = alloc(RememberStatement);

  header.Type = StatementType_Remember;
  ret->Statement = header;

  ret->Variables = variables;

  return ret;
}

GerundList new_GerundList()
{
  GerundList ret;

  ret.First = ret.Last = NULL;

  return ret;
}

void GerundList_Add(GerundList *list, Gerund item)
{
  GerundListNode *new_node = alloc(GerundListNode);

  new_node->Previous = list->Last;
  new_node->Next = NULL;
  new_node->This = item;

  if (list->Last)
    list->Last->Next = new_node;

  if (!list->First)
    list->First = new_node;
  list->Last = new_node;
}

int GerundList_Count(GerundList *list)
{
  GerundListNode *trace = list->First;
  int count = (trace != NULL);

  while (trace != list->Last)
    count++, trace = trace->Next;

  return count;
}

void GerundList_Unlink(GerundList *list, GerundListNode *node)
{
  if (list->First == node)
    list->First = list->First->Next;
  if (list->Last == node)
    list->Last = list->Last->Previous;

  if (node->Previous && (node->Previous->Next == node))
    node->Previous->Next = node->Next;
  if (node->Next && (node->Next->Previous == node))
    node->Next->Previous = node->Previous;

  node->Previous = node->Next = NULL;

  free(node);
}

AbstainStatement *new_AbstainStatement(Statement header, GerundList gerunds, ExpressionList labels)
{
  AbstainStatement *ret = alloc(AbstainStatement);

  header.Type = StatementType_Abstain;
  ret->Statement = header;

  ret->Gerunds = gerunds;
  ret->Labels = labels;

  return ret;
}

ReinstateStatement *new_ReinstateStatement(Statement header, GerundList gerunds, ExpressionList labels)
{
  ReinstateStatement *ret = alloc(ReinstateStatement);

  header.Type = StatementType_Reinstate;
  ret->Statement = header;

  ret->Gerunds = gerunds;
  ret->Labels = labels;

  return ret;
}

GiveUpStatement *new_GiveUpStatement(Statement header)
{
  GiveUpStatement *ret = alloc(GiveUpStatement);

  header.Type = StatementType_GiveUp;
  ret->Statement = header;

  return ret;
}

WriteInStatement *new_WriteInStatement(Statement header, ExpressionList targets)
{
  WriteInStatement *ret = alloc(WriteInStatement);

  header.Type = StatementType_WriteIn;
  ret->Statement = header;

  ret->Targets = targets;

  return ret;
}

ReadOutStatement *new_ReadOutStatement(Statement header, ExpressionList sources)
{
  ReadOutStatement *ret = alloc(ReadOutStatement);

  header.Type = StatementType_ReadOut;
  ret->Statement = header;

  ret->Sources = sources;

  return ret;
}

ComeFromStatement *new_ComeFromStatement(Statement header, ushort label)
{
  ComeFromStatement *ret = alloc(ComeFromStatement);

  header.Type = StatementType_ComeFrom;
  ret->Statement = header;

  ret->Label = label;

  return ret;
}

