#include <stdlib.h>

#include "interpret.h"
#include "program.h"
#include "fuckup.h"
#include "types.h"
#include "io.h"

extern bool strict_call_stack_size;
extern bool cheat_for_syslib_functions;

typedef struct sLabelMap
{
  struct sLabelMap *Left, *Right;

  ushort Label;
  StatementListNode *Statement;
} LabelMap;

int labelmap_ptr_comparator(const void *left, const void *right)
{
  LabelMap **left_ptr = (LabelMap **)left;
  LabelMap **right_ptr = (LabelMap **)right;

  LabelMap *left_map = *left_ptr;
  LabelMap *right_map = *right_ptr;

  return left_map->Label - right_map->Label;
}

static void compile_label_map_list(StatementList program, LabelMap ***labels, int *label_count, int *allocated_labels)
{
  StatementListNode *trace = program.First;

  *label_count = 0;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    if (statement->Label != 0)
    {
      LabelMap *new_map = alloc(LabelMap);

      if (*label_count >= *allocated_labels)
      {
        int new_allocated_labels = *allocated_labels * 2;
        LabelMap **new_labels = malloc(new_allocated_labels * sizeof(LabelMap *));

        memcpy(new_labels, *labels, *allocated_labels * sizeof(LabelMap *));
        free(*labels);

        *labels = new_labels;
        *allocated_labels = new_allocated_labels;
      }

      new_map->Label = statement->Label;
      new_map->Statement = trace;

      (*labels)[*label_count] = new_map;

      ++*label_count;
    }

    trace = trace->Next;
  }

  qsort(*labels, *label_count, sizeof(LabelMap *), labelmap_ptr_comparator);
}

static LabelMap *make_label_map_tree(LabelMap **labels, int num_labels)
{
  int root_index = num_labels / 2;

  if (num_labels == 0)
    return NULL;

  labels[root_index]->Left = make_label_map_tree(labels, root_index);
  labels[root_index]->Right = make_label_map_tree(labels + (root_index + 1), num_labels - (root_index + 1));

  return labels[root_index];
}

static StatementListNode *lookup_label(ushort label, LabelMap *tree)
{
  while (tree)
  {
    if (label == tree->Label)
      return tree->Statement;

    if (label < tree->Label)
      tree = tree->Left;
    else
      tree = tree->Right;
  }

  return NULL;
}

static void assign_call_from_suck_points(StatementList program, LabelMap *labels_tree_root)
{
  StatementListNode *trace = program.First;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    if (statement->Type == StatementType_ComeFrom)
    {
      ComeFromStatement *come_from = (ComeFromStatement *)statement;

      StatementListNode *statement_for_label = lookup_label(come_from->Label, labels_tree_root);

      if (statement_for_label == NULL)
        complain(444, error_code_to_string(444), NULL, 0, 0);

      if (statement_for_label->This->SuckPointDestination != NULL)
        complain(555, error_code_to_string(555), NULL, 0, 0);

      statement_for_label->This->SuckPointDestination = trace;
    }

    trace = trace->Next;
  }
}

static void sort_statements(StatementList program, StatementList statements_by_type[NumGerunds])
{
  StatementListNode *trace;
  int i;

  for (i=0; i < NumGerunds; i++)
    statements_by_type[i] = new_StatementList();

  trace = program.First;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    switch (statement->Type)
    {
      case StatementType_Assignment: StatementList_Add(&statements_by_type[Gerund_Assigning], statement);    break;
      case StatementType_Next:       StatementList_Add(&statements_by_type[Gerund_Nexting], statement);      break;
      case StatementType_Resume:     StatementList_Add(&statements_by_type[Gerund_Resuming], statement);     break;
      case StatementType_Forget:     StatementList_Add(&statements_by_type[Gerund_Forgetting], statement);   break;
      case StatementType_Stash:      StatementList_Add(&statements_by_type[Gerund_Stashing], statement);     break;
      case StatementType_Retrieve:   StatementList_Add(&statements_by_type[Gerund_Retrieving], statement);   break;
      case StatementType_Ignore:     StatementList_Add(&statements_by_type[Gerund_Ignoring], statement);     break;
      case StatementType_Remember:   StatementList_Add(&statements_by_type[Gerund_Remembering], statement);  break;
      case StatementType_Abstain:    StatementList_Add(&statements_by_type[Gerund_Abstaining], statement);   break;
      case StatementType_Reinstate:  StatementList_Add(&statements_by_type[Gerund_Reinstating], statement);  break;
      case StatementType_WriteIn:    StatementList_Add(&statements_by_type[Gerund_WritingIn], statement);    break;
      case StatementType_ReadOut:    StatementList_Add(&statements_by_type[Gerund_ReadingOut], statement);   break;
      case StatementType_ComeFrom:   StatementList_Add(&statements_by_type[Gerund_ComingFrom], statement);   break;
    }

    trace = trace->Next;
  }
}

typedef union uArrayData
{
  ushort *TailData;
  uint *HybridData;
} ArrayData;

typedef struct sArray
{
  int NumDimensions;
  int *Dimensions;
  ArrayData Data;
} Array;

typedef struct sVariables
{
  ushort OneSpots[65536];
  uint TwoSpots[65536];
  Array *Tails[65536];
  Array *Hybrids[65536];

  uint OneSpotsIgnored[65536 / 32];
  uint TwoSpotsIgnored[65536 / 32];
  uint TailsIgnored[65536 / 32];
  uint HybridsIgnored[65536 / 32];
} Variables;

ushort mingle_precalc[256][256];

static void initialize_mingle_precalc()
{
  int left, right;

  for (left=0; left < 256; left++)
    for (right=0; right < 256; right++)
    {
      int mingled = 0, l = left, r = right;
      int bit;

      for (bit=0; bit < 8; bit++)
      {
        mingled = (mingled << 1) | ((l & 128) >> 7); l <<= 1;
        mingled = (mingled << 1) | ((r & 128) >> 7); r <<= 1;
      }

      mingle_precalc[left][right] = mingled;
    }
}

typedef struct sValue
{
  bool FullWidth;
  uint Value;
  Array *Array;
  struct sValue *Next;
} Value;

static Value evaluate_expression(Expression *expr, Variables *variables)
{
  Value value = { 0 };

  switch (expr->Type)
  {
    case ExpressionType_Unary:
    {
      UnaryExpression *expression = (UnaryExpression *)expr;
      int top_bit;
      uint opposite;

      value = evaluate_expression(expression->InnerExpression, variables);

      if ((value.Next != NULL) || (value.Array != NULL))
        complain(300, error_code_to_string(300), NULL, 0, 0);

      if (value.FullWidth)
        top_bit = 0x80000000;
      else
        top_bit = 0x8000;

      opposite = (value.Value >> 1);
      if ((value.Value & 1) != 0)
        opposite |= top_bit;

      switch (expression->Operator)
      {
        case UnaryOperator_And: value.Value &= opposite; break;
        case UnaryOperator_Or:  value.Value |= opposite; break;
        case UnaryOperator_XOr: value.Value ^= opposite; break;
      }

      break;
    }
    case ExpressionType_Binary:
    {
      BinaryExpression *expression = (BinaryExpression *)expr;

      Value left = evaluate_expression(expression->LeftOperand, variables);
      Value right = evaluate_expression(expression->RightOperand, variables);

      if ((left.Next != NULL)  || (left.Array != NULL)
       || (right.Next != NULL) || (right.Array != NULL))
        complain(300, error_code_to_string(300), NULL, 0, 0);

      switch (expression->Operator)
      {
        case BinaryOperator_Interleave:
        {
          if ((left.FullWidth && (left.Value > 0xFFFF))
           || (right.FullWidth && (right.Value > 0xFFFF)))
            complain(276, "YOU WANT I SHOULD ADD SIXTY FOUR BIT VARIABLES, MM?", NULL, 0, 0);

          value.FullWidth = true;
          value.Value = mingle_precalc[left.Value & 0xFF][right.Value & 0xFF]
                     | (mingle_precalc[(left.Value >> 8) & 0xFF][(right.Value >> 8) & 0xFF] << 16);

          break;
        }
        case BinaryOperator_Select:
        {
          int num_bits = 0;
          uint bit_value = 1;

          while (right.Value != 0)
          {
            if ((right.Value & 1) != 0)
            {
              if ((left.Value & 1) != 0)
                value.Value |= bit_value;

              num_bits++;
              bit_value <<= 1;
            }

            left.Value >>= 1;
            right.Value >>= 1;
          }

          value.FullWidth = (num_bits > 16);

          break;
        }
      }

      break;
    }
    case ExpressionType_Subscript:
    {
      SubscriptExpression *expression = (SubscriptExpression *)expr;

      ImmediateExpression *array = (ImmediateExpression *)expression->Array;

      int index = array->Index;

      switch (array->Type)
      {
        case ImmediateType_Tail:
        {
          int offset, scale, dimension_index;
          ExpressionListNode *trace;
          Array *array;

          array = variables->Tails[index];

          if (array == NULL)
            complain(583, error_code_to_string(583), NULL, 0, 0);

          offset = 0;
          scale = 1;
          trace = expression->Subscripts.First;
          dimension_index = 0;
          while (trace != NULL)
          {
            Value subscript_value;

            if (dimension_index >= array->NumDimensions)
              complain(241, error_code_to_string(241), NULL, 0, 0);

            subscript_value = evaluate_expression(trace->This, variables);

            if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
              complain(241, error_code_to_string(241), NULL, 0, 0);

            offset += (subscript_value.Value - 1) * scale;
            scale *= array->Dimensions[dimension_index];
            dimension_index++;

            trace = trace->Next;
          }

          if (dimension_index < array->NumDimensions)
            complain(241, error_code_to_string(241), NULL, 0, 0);

          value.Value = array->Data.TailData[offset];

          break;
        }
        case ImmediateType_Hybrid:
        {
          int offset, scale, dimension_index;
          ExpressionListNode *trace;
          Array *array;

          array = variables->Hybrids[index];

          if (array == NULL)
            complain(583, error_code_to_string(583), NULL, 0, 0);

          offset = 0;
          scale = 1;
          trace = expression->Subscripts.First;
          dimension_index = 0;
          while (trace != NULL)
          {
            Value subscript_value;

            if (dimension_index >= array->NumDimensions)
              complain(241, error_code_to_string(241), NULL, 0, 0);

            subscript_value = evaluate_expression(trace->This, variables);

            if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
              complain(241, error_code_to_string(241), NULL, 0, 0);

            offset += (subscript_value.Value - 1) * scale;
            scale *= array->Dimensions[dimension_index];
            dimension_index++;

            trace = trace->Next;
          }

          if (dimension_index < array->NumDimensions)
            complain(241, error_code_to_string(241), NULL, 0, 0);

          value.Value = array->Data.HybridData[offset];
          value.FullWidth = true;

          break;
        }
      }

      break;
    }
    case ExpressionType_Dimension:
    {
      DimensionExpression *expression = (DimensionExpression *)expr;

      ExpressionListNode *expr_trace = expression->Sizes.First;
      Value *value_trace = &value;

      while (expr_trace != NULL)
      {
        *value_trace = evaluate_expression(expr_trace->This, variables);

        if ((value_trace->Next != NULL) || (value_trace->Array != NULL))
          complain(300, error_code_to_string(300), NULL, 0, 0);

        expr_trace = expr_trace->Next;

        if (expr_trace != NULL)
        {
          Value *new_value = alloc(Value);

          memset(new_value, 0, sizeof(Value));

          value_trace->Next = new_value;
          value_trace = new_value;
        }
      }

      break;
    }
    case ExpressionType_Immediate:
    {
      ImmediateExpression *expression = (ImmediateExpression *)expr;

      int index = expression->Index;

      switch (expression->Type)
      {
        case ImmediateType_OneSpot: value.Value = variables->OneSpots[index];                         break;
        case ImmediateType_TwoSpot: value.Value = variables->TwoSpots[index]; value.FullWidth = true; break;
        case ImmediateType_Tail:    value.Array = variables->Tails[index];                            break;
        case ImmediateType_Hybrid:  value.Array = variables->Hybrids[index];  value.FullWidth = true; break;
        case ImmediateType_Mesh:    value.Value = index; break;
      }

      switch (expression->Type)
      {
        case ImmediateType_Tail:
        case ImmediateType_Hybrid:
          if (value.Array == NULL)
            complain(583, error_code_to_string(583), NULL, 0, 0);
      }

      break;
    }
  }

  if (!value.FullWidth)
    value.Value &= 0xFFFF;

  return value;
}

typedef struct sVariableStash
{
  struct sVariableStash *Previous;
  uint Value;
} VariableStash;

typedef struct sArrayStash
{
  struct sArrayStash *Previous;
  Array *Value;
} ArrayStash;

typedef struct sStashSpace
{
  VariableStash *OneSpots[65536];
  VariableStash *TwoSpots[65536];
  ArrayStash *Tails[65536];
  ArrayStash *Hybrids[65536];
} StashSpace;

static int count_values(Value value)
{
  int count = 1;

  Value *trace = value.Next;

  while (trace != NULL)
  {
    count++;
    trace = trace->Next;
  }

  return count;
}

static void free_array(Array *array)
{
  if (array != NULL)
  {
    free(array->Dimensions);
    free(array->Data.TailData);
    free(array);
  }
}

static void init_array(Variables *variables, int index, int num_bits, Value value)
{
  Array *array, **ptr;
  int i;
  Value *trace;
  int num_elements;

  if (num_bits == 16)
    ptr = &variables->Tails[index];
  else
    ptr = &variables->Hybrids[index];

  array = alloc(Array);

  array->NumDimensions = count_values(value);
  array->Dimensions = malloc(array->NumDimensions * sizeof(int));

  trace = &value;

  num_elements = 1;

  for (i=0; i < array->NumDimensions; i++)
  {
    array->Dimensions[i] = trace->Value;
    num_elements *= array->Dimensions[i];
    trace = trace->Next;
  }

  if (num_elements == 0)
    complain(240, error_code_to_string(240), NULL, 0, 0);

  switch (num_bits)
  {
    case 16:
      array->Data.TailData = malloc(num_elements * sizeof(ushort));
      memset(array->Data.TailData, 0, num_elements * sizeof(ushort));
      break;
    case 32:
      array->Data.HybridData = malloc(num_elements * sizeof(uint));
      memset(array->Data.HybridData, 0, num_elements * sizeof(uint));
      break;
  }

  free_array(*ptr);
  *ptr = array;
}

static Array *copy_array(Array *other, int num_bits)
{
  Array *ret = alloc(Array);
  int i, num_elements;

  ret->NumDimensions = other->NumDimensions;
  ret->Dimensions = malloc(ret->NumDimensions * sizeof(int));

  memcpy(ret->Dimensions, other->Dimensions, ret->NumDimensions * sizeof(int));

  num_elements = 1;
  for (i=0; i < ret->NumDimensions; i++)
    num_elements *= ret->Dimensions[i];

  switch (num_bits)
  {
    case 16:
      ret->Data.TailData = malloc(num_elements * sizeof(ushort));
      memcpy(ret->Data.TailData, other->Data.TailData, num_elements * sizeof(ushort));
      break;
    case 32:
      ret->Data.HybridData = malloc(num_elements * sizeof(uint));
      memcpy(ret->Data.HybridData, other->Data.HybridData, num_elements * sizeof(uint));
      break;
  }

  return ret;
}

static Array *translate_copy_array(Array *other, int from_bits, int to_bits)
{
  Array *ret = alloc(Array);
  int i, num_elements;

  ret->NumDimensions = other->NumDimensions;
  ret->Dimensions = malloc(ret->NumDimensions * sizeof(int));

  memcpy(ret->Dimensions, other->Dimensions, ret->NumDimensions * sizeof(int));

  num_elements = 1;
  for (i=0; i < ret->NumDimensions; i++)
    num_elements *= ret->Dimensions[i];

  switch (to_bits)
  {
    case 16: ret->Data.TailData = malloc(num_elements * sizeof(ushort)); break;
    case 32: ret->Data.HybridData = malloc(num_elements * sizeof(uint)); break;
  }

  for (i=0; i < num_elements; i++)
  {
    uint value;

    switch (from_bits)
    {
      case 16: value = other->Data.TailData[i]; break;
      case 32: value = other->Data.HybridData[i]; break;
    }

    switch (to_bits)
    {
      case 16:
        if (value > 0xFFFF)
          complain(275, "THE PROGRAMMER HAS JOHNNY COCHRANNED A TAIL", NULL, 0, 0);
        ret->Data.TailData[i] = value;
        break;
      case 32:
        ret->Data.HybridData[i] = value;
        break;
    }
  }

  return ret;
}

static void free_value(Value value)
{
  Value *trace = value.Next;

  while (trace != NULL)
  {
    Value *next = trace->Next;

    free(trace);
    trace = next;
  }

  value.Next = NULL;
}

typedef struct sCallFrame
{
  StatementListNode *ContinueAfterStatement;
} CallFrame;

void check_for_user_syslib_overrides(StatementList program, bool do_fast_syslib_for_label[2000])
{
  StatementListNode *trace;

  memset(do_fast_syslib_for_label, 0, 2000 * sizeof(bool));

  do_fast_syslib_for_label[1000] = do_fast_syslib_for_label[1009]
    = do_fast_syslib_for_label[1010] = do_fast_syslib_for_label[1020]
    = do_fast_syslib_for_label[1030] = do_fast_syslib_for_label[1039]
    = do_fast_syslib_for_label[1040] = do_fast_syslib_for_label[1050]
    = do_fast_syslib_for_label[1060] = do_fast_syslib_for_label[1070]
    = do_fast_syslib_for_label[1080] = do_fast_syslib_for_label[1500]
    = do_fast_syslib_for_label[1509] = do_fast_syslib_for_label[1510]
    = do_fast_syslib_for_label[1520] = do_fast_syslib_for_label[1525]
    = do_fast_syslib_for_label[1530] = do_fast_syslib_for_label[1540]
    = do_fast_syslib_for_label[1549] = do_fast_syslib_for_label[1550]
    = do_fast_syslib_for_label[1900] = do_fast_syslib_for_label[1910] = true;

  trace = program.First;

  while (trace != NULL)
  {
    Statement *statement = trace->This;

    if (statement->Label < 2000)
      do_fast_syslib_for_label[statement->Label] = false;

    trace = trace->Next;
  }
}

void interpret(StatementList program)
{
  int allocated_labels = 100;
  LabelMap **labels = malloc(allocated_labels * sizeof(LabelMap *));
  int num_labels = 0;
  LabelMap *label_tree_root;
  StatementList statements_by_type[NumGerunds];
  StatementListNode *current_statement;
  bool abstenance_list[NumGerunds] = { false };
  Variables *variables = alloc(Variables); // <-- a bit under 1 megabyte!
  StashSpace *stash_space = alloc(StashSpace); // <-- another megabyte!
  int call_stack_size = 79, call_depth = 0;
  CallFrame *call_stack = malloc(call_stack_size * sizeof(CallFrame));
  bool do_fast_syslib_for_label[2000];

  memset(variables, 0, sizeof(Variables));
  memset(stash_space, 0, sizeof(StashSpace));

  initialize_mingle_precalc();

  compile_label_map_list(program, &labels, &num_labels, &allocated_labels);
  label_tree_root = make_label_map_tree(labels, num_labels);
  assign_call_from_suck_points(program, label_tree_root);
  sort_statements(program, statements_by_type);
  check_for_user_syslib_overrides(program, do_fast_syslib_for_label);

  current_statement = program.First;

  while (current_statement != NULL)
  {
    bool run_statement = true;

    if (current_statement->This->Probability != 100)
    {
      if (current_statement->This->Probability <= 0)
        run_statement = false;
      else
        run_statement = (rand() % 100) < current_statement->This->Probability;
    }

    if (run_statement)
      switch (current_statement->This->Type)
      {
        case StatementType_Bad:
        {
          BadStatement *statement = (BadStatement *)current_statement->This;

          complain(
            current_statement->This->ErrorCode,
            error_code_to_string(current_statement->This->ErrorCode),
            statement->Text,
            0, 0);
        }
        case StatementType_Assignment:
        {
          AssignmentStatement *statement = (AssignmentStatement *)current_statement->This;

          Value value = { 0 };

          bool size_32_bits = false;
          
          if (abstenance_list[Gerund_Calculating] == false)
          {
            value = evaluate_expression(statement->Value, variables);

            if (value.FullWidth && (value.Value > 0xFFFF))
              size_32_bits = true;
          }

          if (abstenance_list[Gerund_Assigning] == false)
          {
            switch (statement->Target->Type)
            {
              case ExpressionType_Immediate:
              {
                ImmediateExpression *target = (ImmediateExpression *)statement->Target;
                int index = target->Index;

                switch (target->Type)
                {
                  case ImmediateType_OneSpot:
                    if (size_32_bits)
                      complain(275, error_code_to_string(275), NULL, 0, 0);

                    if ((value.Next != NULL) || (value.Array != NULL))
                      complain(300, error_code_to_string(300), NULL, 0, 0);

                    if (0 == (variables->OneSpotsIgnored[index / 32] & (1 << (index % 31))))
                      variables->OneSpots[index] = value.Value;

                    break;
                  case ImmediateType_TwoSpot:
                    if ((value.Next != NULL) || (value.Array != NULL))
                      complain(300, error_code_to_string(300), NULL, 0, 0);

                    if (0 == (variables->TwoSpotsIgnored[index / 32] & (1 << (index & 31))))
                      variables->TwoSpots[index] = value.Value;

                    break;
                  case ImmediateType_Tail:
                    if (0 == (variables->TailsIgnored[index / 32] & (1 << (index & 31))))
                      if (value.Array != NULL)
                      {
                        free_array(variables->Tails[index]);
                        if (value.FullWidth)
                          variables->Tails[index] = translate_copy_array(value.Array, 32, 16);
                        else
                          variables->Tails[index] = copy_array(value.Array, 16);
                      }
                      else
                        init_array(variables, index, 16, value);

                    break;
                  case ImmediateType_Hybrid:
                    if (0 == (variables->HybridsIgnored[index / 32] & (1 << (index & 31))))
                      if (value.Array != NULL)
                      {
                        free_array(variables->Hybrids[index]);
                        if (value.FullWidth)
                          variables->Hybrids[index] = copy_array(value.Array, 32);
                        else
                          variables->Hybrids[index] = translate_copy_array(value.Array, 16, 32);
                      }
                      else
                        init_array(variables, index, 32, value);

                    break;
                }

                break;
              }
              case ExpressionType_Subscript:
              {
                SubscriptExpression *target = (SubscriptExpression *)statement->Target;

                ImmediateExpression *array = (ImmediateExpression *)target->Array;

                int index = array->Index;

                if ((value.Next != NULL) || (value.Array != NULL))
                  complain(300, error_code_to_string(300), NULL, 0, 0);

                switch (array->Type)
                {
                  case ImmediateType_Tail:
                    if (size_32_bits)
                      complain(275, error_code_to_string(275), NULL, 0, 0);

                    if (0 == (variables->TailsIgnored[index / 32] & (1 << (index & 31))))
                    {
                      int offset, scale, dimension_index;
                      ExpressionListNode *trace;
                      Array *array;

                      array = variables->Tails[index];

                      if (array == NULL)
                        complain(583, error_code_to_string(583), NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = target->Subscripts.First;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        Value subscript_value;

                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        subscript_value = evaluate_expression(trace->This, variables);

                        if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += (subscript_value.Value - 1) * scale;
                        scale *= array->Dimensions[dimension_index];
                        dimension_index++;

                        trace = trace->Next;
                      }

                      if (dimension_index < array->NumDimensions)
                        complain(241, error_code_to_string(241), NULL, 0, 0);

                      array->Data.TailData[offset] = value.Value;
                    }

                    break;
                  case ImmediateType_Hybrid:
                    if (0 == (variables->HybridsIgnored[index / 32] & (1 << (index & 31))))
                    {
                      int offset, scale, dimension_index;
                      ExpressionListNode *trace;
                      Array *array;

                      array = variables->Hybrids[index];

                      if (array == NULL)
                        complain(583, error_code_to_string(583), NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = target->Subscripts.First;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        Value subscript_value;

                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        subscript_value = evaluate_expression(trace->This, variables);

                        if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += (subscript_value.Value - 1) * scale;
                        scale *= array->Dimensions[dimension_index];
                        dimension_index++;

                        trace = trace->Next;
                      }

                      if (dimension_index < array->NumDimensions)
                        complain(241, error_code_to_string(241), NULL, 0, 0);

                      array->Data.HybridData[offset] = value.Value;
                    }

                    break;
                }

                break;
              }
            }
          }

          break;
        }
        case StatementType_Next:
        {
          NextStatement *statement = (NextStatement *)current_statement->This;

          if (abstenance_list[Gerund_Nexting] == true)
            break;

          if (cheat_for_syslib_functions
           && (statement->Label < 2000)
           && do_fast_syslib_for_label[statement->Label])
            switch (statement->Label)
            {
              case 1000: // .3 <- .1 + .2
              {
                if (variables->OneSpots[1] > ~variables->OneSpots[2])
                  complain(0, "%s", "(1999)  DOUBLE OR SINGLE PRECISION OVERFLOW\n", 0, 0);

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] + variables->OneSpots[2];

                break;
              }
              case 1009: // .3 <- unchecked(.1 + .2), .4 <- (overflow == false) ? #1 : #2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 16))
                  if (variables->OneSpots[1] > ~variables->OneSpots[2])
                    variables->OneSpots[4] = 2;
                  else
                    variables->OneSpots[4] = 1;

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] + variables->OneSpots[2];

                break;
              }
              case 1010: // .3 <- .1 - .2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] - variables->OneSpots[2];

                break;
              }
              case 1020: // .1++
              {
                if (0 == (variables->OneSpotsIgnored[0] & 2))
                  variables->OneSpots[1]++;

                break;
              }
              case 1030: // .3 <- .1 * .2
              {
                if (variables->OneSpots[1] > 0xFFFFFFFFu / variables->OneSpots[2])
                  complain(0, "%s", "(1999)  DOUBLE OR SINGLE PRECISION OVERFLOW\n", 0, 0);

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] * variables->OneSpots[2];

                break;
              }
              case 1039: // .3 <- unchecked(.1 * .2), .4 <- (overflow == false) ? #1 : #2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 16))
                  if (variables->OneSpots[1] > 0xFFFFu / variables->OneSpots[2])
                    variables->OneSpots[4] = 2;
                  else
                    variables->OneSpots[4] = 1;

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] * variables->OneSpots[2];

                break;
              }
              case 1040: // .3 <- (.2 != 0) ? .1 / .2 : #0
              {
                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  if (variables->OneSpots[2] != 0)
                    variables->OneSpots[3] = variables->OneSpots[1] / variables->OneSpots[2];
                  else
                    variables->OneSpots[3] = 0;

                break;
              }
              case 1050: // .2 <- (.1 != 0) ? :1 / .1 : #0
              {
                if (0 == (variables->OneSpotsIgnored[0] & 4))
                  if (variables->OneSpots[1] != 0)
                    variables->OneSpots[2] = variables->TwoSpots[1] / variables->OneSpots[1];
                  else
                    variables->OneSpots[2] = 0;

                break;
              }
              case 1060: // .3 <- .1 | .2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] | variables->OneSpots[2];

                break;
              }
              case 1070: // .3 <- .1 & .2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] & variables->OneSpots[2];

                break;
              }
              case 1080: // .3 <- .1 ^ .2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] ^ variables->OneSpots[2];

                break;
              }
              case 1500: // :3 <- :1 + :2
              {
                if (variables->TwoSpots[1] > ~variables->TwoSpots[2])
                  complain(0, "%s", "(1999)  DOUBLE OR SINGLE PRECISION OVERFLOW\n", 0, 0);

                if (0 == (variables->TwoSpotsIgnored[0] & 8))
                  variables->TwoSpots[3] = variables->TwoSpots[1] + variables->TwoSpots[2];

                break;
              }
              case 1509: // :3 <- unchecked(:1 + :2), :4 <- (overflow == false) ? #1 : #2
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 16))
                  if (variables->TwoSpots[1] > ~variables->TwoSpots[2])
                    variables->TwoSpots[4] = 2;
                  else
                    variables->TwoSpots[4] = 1;

                if (0 == (variables->TwoSpotsIgnored[0] & 8))
                  variables->TwoSpots[3] = variables->TwoSpots[1] + variables->TwoSpots[2];

                break;
              }
              case 1510: // :3 <- :1 - :2
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 8))
                  variables->TwoSpots[3] = variables->TwoSpots[1] - variables->TwoSpots[2];

                break;
              }
              case 1520: // :1 <- (.1 << 16) | .2
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 2))
                  variables->TwoSpots[1] = (variables->OneSpots[1] << 16) | variables->OneSpots[2];

                break;
              }
              case 1525: // ("undocumented") .3 <<= 8
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[3] << 8;

                break;
              }
              case 1530: // :1 <- .1 * .2
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 2))
                  variables->TwoSpots[1] = variables->OneSpots[1] * variables->OneSpots[2];

                break;
              }
              case 1540: // :3 <- :1 * :2
              {
                if (variables->OneSpots[1] > 0xFFFFu / variables->OneSpots[2])
                  complain(0, "%s", "(1999)  DOUBLE OR SINGLE PRECISION OVERFLOW\n", 0, 0);

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] * variables->OneSpots[2];

                break;
              }
              case 1549: // :3 <- unchecked(:1 * :2), .4 <- (overflow == false) ? #1 : #2
              {
                if (0 == (variables->OneSpotsIgnored[0] & 16))
                  if (variables->OneSpots[1] > 0xFFFFFFFFu / variables->OneSpots[2])
                    variables->OneSpots[4] = 2;
                  else
                    variables->OneSpots[4] = 1;

                if (0 == (variables->OneSpotsIgnored[0] & 8))
                  variables->OneSpots[3] = variables->OneSpots[1] * variables->OneSpots[2];

                break;
              }
              case 1550: // :3 <- (:2 != 0) ? :1 / :2 : #0
              {
                if (0 == (variables->TwoSpotsIgnored[0] & 8))
                  if (variables->TwoSpots[2] != 0)
                    variables->TwoSpots[3] = variables->TwoSpots[1] / variables->TwoSpots[2];
                  else
                    variables->TwoSpots[3] = 0;

                break;
              }
              case 1900: // .1 <- uniform random no. from #0 to #65535
              {
                if (0 == (variables->OneSpotsIgnored[0] & 2))
                {
#if RAND_MAX > 0xFFFF
                  variables->OneSpots[1] = ((long long)rand()) * 65535LL / (1LL + (long long)RAND_MAX);
#else
                  int low_half = rand() * 256 / (RAND_MAX + 1);
                  int high_half = rand() * 256 / (RAND_MAX + 1);

                  variables->OneSpots[1] = low_half | (high_half << 8);
#endif /* RAND_MAX */
                }

                break;
              }
              case 1910: // .2 <- normal random no. from #0 to .1, with standard deviation .1 divided by #12
              {
                int i;
#if RAND_MAX > 0xFFFF
                long long accumulator;
#else
                uint accumulator;
#endif /* RAND_MAX */

                accumulator = 0;
                for (i=0; i < 12; i++)
                  accumulator += rand();

                accumulator = accumulator * variables->OneSpots[1] / 12 + (RAND_MAX / 2);

                if (0 == (variables->OneSpotsIgnored[0] & 4))
                  variables->OneSpots[2] = accumulator / RAND_MAX;

                break;
              }
            }

          if (call_depth == call_stack_size)
            if (strict_call_stack_size)
              complain(123, error_code_to_string(123), NULL, 0, 0);
            else
            {
              int new_call_stack_size = call_stack_size * 2;
              CallFrame *new_call_stack = malloc(new_call_stack_size * sizeof(CallFrame));

              memcpy(new_call_stack, call_stack, call_stack_size * sizeof(CallFrame));
              free(call_stack);

              call_stack = new_call_stack;
              call_stack_size = new_call_stack_size;
            }

          call_stack[call_depth++].ContinueAfterStatement = current_statement;

          current_statement = lookup_label(statement->Label, label_tree_root);

          if (current_statement == NULL)
            complain(129, error_code_to_string(129), NULL, 0, 0);

          continue;
        }
        case StatementType_Resume:
        {
          ResumeStatement *statement = (ResumeStatement *)current_statement->This;

          Value count = { 0 };

          if (abstenance_list[Gerund_Calculating] == false)
            count = evaluate_expression(statement->Count, variables);

          if ((count.Next != NULL) || (count.Array != NULL))
            complain(300, error_code_to_string(300), NULL, 0, 0);

          if (abstenance_list[Gerund_Resuming] == true)
            break;

          if (count.Value == 0)
            complain(621, error_code_to_string(621), NULL, 0, 0);

          if (count.Value > (uint)call_depth)
            complain(632, error_code_to_string(632), NULL, 0, 0);

          call_depth -= count.Value;

          current_statement = call_stack[call_depth].ContinueAfterStatement;

          break;
        }
        case StatementType_Forget:
        {
          ForgetStatement *statement = (ForgetStatement *)current_statement->This;

          Value count = { 0 };

          if (abstenance_list[Gerund_Calculating] == false)
            count = evaluate_expression(statement->Count, variables);

          if ((count.Next != NULL) || (count.Array != NULL))
            complain(300, error_code_to_string(300), NULL, 0, 0);

          if (abstenance_list[Gerund_Forgetting] == true)
            break;

          call_depth -= count.Value;
          if (call_depth < 0)
            call_depth = 0;

          break;
        }
        case StatementType_Stash:
        {
          StashStatement *statement = (StashStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_Stashing] == true)
            break;

          trace = statement->Variables.First;
          while (trace != NULL)
          {
            ImmediateExpression *variable = (ImmediateExpression *)trace->This;

            switch (variable->Type)
            {
              case ImmediateType_OneSpot:
              case ImmediateType_TwoSpot:
              {
                VariableStash **ptr;
                VariableStash *new_stash;
                uint value;

                switch (variable->Type)
                {
                  case ImmediateType_OneSpot:
                    ptr = &stash_space->OneSpots[variable->Index];
                    value = variables->OneSpots[variable->Index];
                    break;
                  case ImmediateType_TwoSpot:
                    ptr = &stash_space->TwoSpots[variable->Index];
                    value = variables->TwoSpots[variable->Index];
                    break;
                }

                new_stash = alloc(VariableStash);

                new_stash->Previous = *ptr;
                new_stash->Value = value;

                *ptr = new_stash;

                break;
              }
              case ImmediateType_Tail:
              case ImmediateType_Hybrid:
              {
                ArrayStash **ptr;
                ArrayStash *new_stash;
                Array *value;
                int num_bits;

                switch (variable->Type)
                {
                  case ImmediateType_Tail:
                    ptr = &stash_space->Tails[variable->Index];
                    value = variables->Tails[variable->Index];
                    num_bits = 16;
                    break;
                  case ImmediateType_Hybrid:
                    ptr = &stash_space->Hybrids[variable->Index];
                    value = variables->Hybrids[variable->Index];
                    num_bits = 32;
                    break;
                }

                if (value == NULL)
                  complain(667, "YOU CAN'T HIDE NOTHING", NULL, 0, 0);

                new_stash = alloc(ArrayStash);

                new_stash->Previous = *ptr;
                new_stash->Value = copy_array(value, num_bits);

                *ptr = new_stash;

                break;
              }
            }

            trace = trace->Next;
          }

          break;
        }
        case StatementType_Retrieve:
        {
          RetrieveStatement *statement = (RetrieveStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_Retrieving] == true)
            break;

          trace = statement->Variables.First;
          while (trace != NULL)
          {
            ImmediateExpression *variable = (ImmediateExpression *)trace->This;

            switch (variable->Type)
            {
              case ImmediateType_OneSpot:
              case ImmediateType_TwoSpot:
              {
                VariableStash **stash_ptr;
                VariableStash *stash;

                switch (variable->Type)
                {
                  case ImmediateType_OneSpot: stash_ptr = &stash_space->OneSpots[variable->Index]; break;
                  case ImmediateType_TwoSpot: stash_ptr = &stash_space->TwoSpots[variable->Index]; break;
                }

                stash = *stash_ptr;

                if (stash == NULL)
                  complain(436, error_code_to_string(436), NULL, 0, 0);

                switch (variable->Type)
                {
                  case ImmediateType_OneSpot:
                    if (0 == (variables->OneSpotsIgnored[variable->Index / 32] & (1 << (variable->Index & 31))))
                      variables->OneSpots[variable->Index] = stash->Value;
                    break;
                  case ImmediateType_TwoSpot:
                    if (0 == (variables->TwoSpotsIgnored[variable->Index / 32] & (1 << (variable->Index & 31))))
                      variables->TwoSpots[variable->Index] = stash->Value;
                    break;
                }

                *stash_ptr = stash->Previous;
                free(stash);

                break;
              }
              case ImmediateType_Tail:
              case ImmediateType_Hybrid:
              {
                ArrayStash **stash_ptr;
                ArrayStash *stash;
                uint *ignored;
                Array **array_ptr;
                int num_bits;

                switch (variable->Type)
                {
                  case ImmediateType_Tail:
                    stash_ptr = &stash_space->Tails[variable->Index];
                    array_ptr = &variables->Tails[variable->Index];
                    ignored = variables->TailsIgnored;
                    num_bits = 16;
                    break;
                  case ImmediateType_Hybrid:
                    stash_ptr = &stash_space->Hybrids[variable->Index];
                    array_ptr = &variables->Hybrids[variable->Index];
                    ignored = variables->HybridsIgnored;
                    num_bits = 32;
                    break;
                }

                stash = *stash_ptr;

                if (stash == NULL)
                  complain(436, error_code_to_string(436), NULL, 0, 0);

                if (0 == (ignored[variable->Index / 32] & (1 << (variable->Index & 31))))
                {
                  free_array(*array_ptr);
                  *array_ptr = stash->Value;
                }
                else
                  free_array(stash->Value);

                *stash_ptr = stash->Previous;
                free(stash);

                break;
              }
            }

            trace = trace->Next;
          }

          break;
        }
        case StatementType_Ignore:
        {
          IgnoreStatement *statement = (IgnoreStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_Ignoring] == true)
            break;

          trace = statement->Variables.First;

          while (trace != NULL)
          {
            ImmediateExpression *variable = (ImmediateExpression *)trace->This;

            int index = variable->Index;

            switch (variable->Type)
            {
              case ImmediateType_OneSpot: variables->OneSpotsIgnored[index / 32] |= (1 << (index & 31)); break;
              case ImmediateType_TwoSpot: variables->TwoSpotsIgnored[index / 32] |= (1 << (index & 31)); break;
              case ImmediateType_Tail:    variables->TailsIgnored   [index / 32] |= (1 << (index & 31)); break;
              case ImmediateType_Hybrid:  variables->HybridsIgnored [index / 32] |= (1 << (index & 31)); break;
            }

            trace = trace->Next;
          }

          break;
        }
        case StatementType_Remember:
        {
          RememberStatement *statement = (RememberStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_Remembering] == true)
            break;

          trace = statement->Variables.First;

          while (trace != NULL)
          {
            ImmediateExpression *variable = (ImmediateExpression *)trace->This;

            int index = variable->Index;

            switch (variable->Type)
            {
              case ImmediateType_OneSpot: variables->OneSpotsIgnored[index / 32] &= ~(1 << (index & 31)); break;
              case ImmediateType_TwoSpot: variables->TwoSpotsIgnored[index / 32] &= ~(1 << (index & 31)); break;
              case ImmediateType_Tail:    variables->TailsIgnored   [index / 32] &= ~(1 << (index & 31)); break;
              case ImmediateType_Hybrid:  variables->HybridsIgnored [index / 32] &= ~(1 << (index & 31)); break;
            }

            trace = trace->Next;
          }

          break;
        }
        case StatementType_Abstain:
        {
          AbstainStatement *statement = (AbstainStatement *)current_statement->This;
          GerundListNode *gerund_trace;
          ExpressionListNode *label_trace;

          if (abstenance_list[Gerund_Abstaining] == true)
            break;

          gerund_trace = statement->Gerunds.First;

          while (gerund_trace != NULL)
          {
            Gerund gerund = gerund_trace->This;

            abstenance_list[gerund] = true;

            gerund_trace = gerund_trace->Next;
          }

          label_trace = statement->Labels.First;

          while (label_trace != NULL)
          {
            ImmediateExpression *expr = (ImmediateExpression *)label_trace->This;
            ushort label = expr->Index;

            StatementListNode *statement = lookup_label(label, label_tree_root);

            if (statement == NULL)
              complain(139, error_code_to_string(139), NULL, 0, 0);

            if (statement->This->Probability > 0)
              statement->This->Probability = -statement->This->Probability;

            label_trace = label_trace->Next;
          }

          break;
        }
        case StatementType_Reinstate:
        {
          ReinstateStatement *statement = (ReinstateStatement *)current_statement->This;
          GerundListNode *gerund_trace;
          ExpressionListNode *label_trace;

          if (abstenance_list[Gerund_Reinstating] == true)
            break;

          gerund_trace = statement->Gerunds.First;

          while (gerund_trace != NULL)
          {
            StatementListNode *trace;
            Gerund gerund = gerund_trace->This;

            abstenance_list[gerund] = false;

            trace = statements_by_type[gerund].First;

            while (trace != NULL)
            {
              Statement *statement = trace->This;

              if ((statement->Type != StatementType_GiveUp)
               && (statement->Probability < 0))
                statement->Probability = -statement->Probability;

              trace = trace->Next;
            }

            gerund_trace = gerund_trace->Next;
          }

          label_trace = statement->Labels.First;

          while (label_trace != NULL)
          {
            ImmediateExpression *expr = (ImmediateExpression *)label_trace->This;
            ushort label = expr->Index;

            StatementListNode *statement = lookup_label(label, label_tree_root);

            if (statement == NULL)
              complain(139, error_code_to_string(139), NULL, 0, 0);

            if (statement->This->Probability < 0)
              statement->This->Probability = -statement->This->Probability;

            label_trace = label_trace->Next;
          }

          break;
        }
        case StatementType_GiveUp:
          return;
        case StatementType_WriteIn:
        {
          WriteInStatement *statement = (WriteInStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_WritingIn] == true)
            break;

          trace = statement->Targets.First;

          while (trace != NULL)
          {
            Expression *target_expression = trace->This;

            switch (target_expression->Type)
            {
              case ExpressionType_Immediate:
              {
                ImmediateExpression *target = (ImmediateExpression *)target_expression;
                int index = target->Index;

                switch (target->Type)
                {
                  case ImmediateType_OneSpot:
                  {
                    uint value = text_in();
                    
                    if (value > 0xFFFF)
                      complain(275, "DON'T BYTE OFF MORE THAN YOU CAN CHEW", NULL, 0, 0);

                    if (0 == (variables->OneSpotsIgnored[index / 32] & (1 << (index % 31))))
                      variables->OneSpots[index] = value;
                    break;
                  }
                  case ImmediateType_TwoSpot:
                  {
                    uint value = text_in();

                    if (0 == (variables->TwoSpotsIgnored[index / 32] & (1 << (index % 31))))
                      variables->TwoSpots[index] = text_in();
                    break;
                  }
                  case ImmediateType_Tail:
                  case ImmediateType_Hybrid:
                  {
                    Array *array;
                    int stride;
                    uint *ignored;
                    void *data;
                    
                    switch (target->Type)
                    {
                      case ImmediateType_Tail:
                        array = variables->Tails[index];
                        stride = 2;
                        data = array->Data.TailData;
                        ignored = variables->TailsIgnored;
                        break;
                      case ImmediateType_Hybrid:
                        array = variables->Hybrids[index];
                        stride = 4;
                        data = array->Data.HybridData;
                        break;
                    }

                    if (array == NULL)
                      complain(583, error_code_to_string(583), NULL, 0, 0);

                    if (array->NumDimensions > 1)
                      complain(241, error_code_to_string(241), NULL, 0, 0);

                    if (0 == (ignored[index / 32] & (1 << (index % 31))))
                    {
                      memset(data, 0, array->Dimensions[0] * stride);
                      binary_in((uchar *)data, array->Dimensions[0], stride);
                    }
                    else
                      binary_skip_in(array->Dimensions[0]);

                    break;
                  }
                }

                break;
              }
              case ExpressionType_Subscript:
              {
                SubscriptExpression *target = (SubscriptExpression *)target_expression;

                ImmediateExpression *array = (ImmediateExpression *)target->Array;

                int index = array->Index;

                uint value = text_in();
                bool size_32_bits = (value > 0xFFFF);

                switch (array->Type)
                {
                  case ImmediateType_Tail:
                    if (size_32_bits)
                      complain(275, error_code_to_string(275), NULL, 0, 0);

                    if (0 == (variables->TailsIgnored[index / 32] & (1 << (index & 31))))
                    {
                      int offset, scale, dimension_index;
                      ExpressionListNode *trace;
                      Array *array;

                      array = variables->Tails[index];

                      if (array == NULL)
                        complain(583, error_code_to_string(583), NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = target->Subscripts.First;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        Value subscript_value;

                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        subscript_value = evaluate_expression(trace->This, variables);

                        if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += (subscript_value.Value - 1) * scale;
                        scale *= array->Dimensions[dimension_index];
                        dimension_index++;

                        trace = trace->Next;
                      }

                      if (dimension_index < array->NumDimensions)
                        complain(241, error_code_to_string(241), NULL, 0, 0);

                      array->Data.TailData[offset] = value;
                    }

                    break;
                  case ImmediateType_Hybrid:
                    if (0 == (variables->HybridsIgnored[index / 32] & (1 << (index & 31))))
                    {
                      int offset, scale, dimension_index;
                      ExpressionListNode *trace;
                      Array *array;

                      array = variables->Hybrids[index];

                      if (array == NULL)
                        complain(583, error_code_to_string(583), NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = target->Subscripts.First;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        Value subscript_value;

                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        subscript_value = evaluate_expression(trace->This, variables);

                        if (subscript_value.Value > (uint)array->Dimensions[dimension_index])
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += (subscript_value.Value - 1) * scale;
                        scale *= array->Dimensions[dimension_index];
                        dimension_index++;

                        trace = trace->Next;
                      }

                      if (dimension_index < array->NumDimensions)
                        complain(241, error_code_to_string(241), NULL, 0, 0);

                      array->Data.HybridData[offset] = value;
                    }

                    break;
                }

                break;
              }
            }

            trace = trace->Next;
          }

          break;
        }
        case StatementType_ReadOut:
        {
          ReadOutStatement *statement = (ReadOutStatement *)current_statement->This;
          ExpressionListNode *trace;

          if (abstenance_list[Gerund_ReadingOut] == true)
            break;

          trace = statement->Sources.First;

          while (trace != NULL)
          {
            Value value = evaluate_expression(trace->This, variables);

            if (value.Next != NULL)
              complain(300, error_code_to_string(300), NULL, 0, 0);

            if (value.Array != NULL)
            {
              int stride;
              void *data;

              if (value.Array->NumDimensions > 1)
                complain(241, error_code_to_string(241), NULL, 0, 0);

              if (value.FullWidth)
              {
                stride = 4;
                data = value.Array->Data.HybridData;
              }
              else
              {
                stride = 2;
                data = value.Array->Data.TailData;
              }

              binary_out(data, value.Array->Dimensions[0], stride);
            }
            else
              text_out(value.Value);

            trace = trace->Next;
          }

          break;
        }
        case StatementType_ComeFrom:
          // do nothing; the action is handled at the suck point
          break;
        default:
          explode("BAD LUCK SWITCH IN MODULE I-I");
      }

    if ((current_statement->This->SuckPointDestination != NULL)
     && (abstenance_list[Gerund_ComingFrom] == false))
      current_statement = current_statement->This->SuckPointDestination;
    else
      current_statement = current_statement->Next;
  }
}
