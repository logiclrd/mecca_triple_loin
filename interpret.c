#include <stdlib.h>

#include "interpret.h"
#include "program.h"
#include "fuckup.h"
#include "types.h"

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
        free(labels);

        *labels = new_labels;
        *allocated_labels = new_allocated_labels;
      }

      new_map->Label = statement->Label;
      new_map->Statement = trace;

      *labels[*label_count] = new_map;

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

typedef struct sValue
{
  bool FullWidth;
  uint Value;
  struct sValue *Next;
} Value;

Value evaluate_expression(Expression *expr)
{
  Value value = { 0 };

  // TODO

  return value;
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

void interpret(StatementList program)
{
  int allocated_labels = 100;
  LabelMap **labels = malloc(allocated_labels * sizeof(LabelMap *));
  int num_labels = 0;
  LabelMap *label_tree_root;
  StatementListNode *current_statement;
  bool abstenance_list[NumGerunds] = { false };
  Variables *variables = alloc(Variables); // <-- a bit under 1 megabyte!
  StashSpace *stash_space = alloc(StashSpace); // <-- another megabyte!
  int call_stack_size = 100, call_depth = 0;
  CallFrame *call_stack = malloc(call_stack_size * sizeof(CallFrame));

  memset(variables, 0, sizeof(Variables));
  memset(stash_space, 0, sizeof(StashSpace));

  compile_label_map_list(program, &labels, &num_labels, &allocated_labels);
  label_tree_root = make_label_map_tree(labels, num_labels);
  assign_call_from_suck_points(program, label_tree_root);

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
            value = evaluate_expression(statement->Value);

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

                switch (target->Type)
                {
                  case ImmediateType_OneSpot:
                  {
                    int index = target->Index;

                    if (size_32_bits)
                      complain(275, error_code_to_string(275), NULL, 0, 0);

                    if (value.Next)
                      complain(300, "CANNOT STUFF DIMENSIONS INTO A BOX", NULL, 0, 0);

                    if (0 == (variables->OneSpotsIgnored[index / 32] & (1 << (index % 31))))
                      variables->OneSpots[index] = value.Value;

                    break;
                  }
                  case ImmediateType_TwoSpot:
                  {
                    int index = target->Index;

                    if (value.Next)
                      complain(300, "CANNOT STUFF DIMENSIONS INTO A BOX", NULL, 0, 0);

                    if (0 == (variables->TwoSpotsIgnored[index / 32] & (1 << (index & 31))))
                      variables->TwoSpots[index] = value.Value;

                    break;
                  }
                  case ImmediateType_Tail:
                  {
                    int index = target->Index;

                    if (0 == (variables->TailsIgnored[index / 32] & (1 << (index & 31))))
                      init_array(variables, index, 16, value);

                    break;
                  }
                  case ImmediateType_Hybrid:
                  {
                    int index = target->Index;

                    if (0 == (variables->HybridsIgnored[index / 32] & (1 << (index & 31))))
                      init_array(variables, index, 32, value);

                    break;
                  }
                }
              }
              case ExpressionType_Subscript:
              {
                SubscriptExpression *target = (SubscriptExpression *)statement->Target;

                ImmediateExpression *array = (ImmediateExpression *)target->Array;

                int index = array->Index;

                if (value.Next != NULL)
                  complain(300, "CANNOT STUFF DIMENSIONS INTO A BOX", NULL, 0, 0);

                switch (array->Type)
                {
                  case ImmediateType_Tail:
                    if (size_32_bits)
                      complain(275, error_code_to_string(275), NULL, 0, 0);

                    if (0 == (variables->TailsIgnored[index / 32] & (1 << (index & 31))))
                    {
                      int offset, scale, dimension_index;
                      Value *trace;
                      Array *array;

                      array = variables->Tails[index];

                      if (array == NULL)
                        complain(583, "THE ARRAY WAS NOT PUT THERE", NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = &value;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += trace->Value * scale;
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
                      Value *trace;
                      Array *array;

                      array = variables->Hybrids[index];

                      if (array == NULL)
                        complain(583, "THE ARRAY WAS NOT PUT THERE", NULL, 0, 0);

                      offset = 0;
                      scale = 1;
                      trace = &value;
                      dimension_index = 0;
                      while (trace != NULL)
                      {
                        if (dimension_index >= array->NumDimensions)
                          complain(241, error_code_to_string(241), NULL, 0, 0);

                        offset += trace->Value * scale;
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

          if (call_depth == call_stack_size)
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
            count = evaluate_expression(statement->Count);

          if (count.Next)
            complain(300, "CANNOT STUFF DIMENSIONS INTO A BOX", NULL, 0, 0);

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
            count = evaluate_expression(statement->Count);

          if (count.Next)
            complain(300, "CANNOT STUFF DIMENSIONS INTO A BOX", NULL, 0, 0);

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
                  case ImmediateType_OneSpot: variables->OneSpots[variable->Index] = stash->Value; break;
                  case ImmediateType_TwoSpot: variables->TwoSpots[variable->Index] = stash->Value; break;
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
                Array **array_ptr;
                int num_bits;

                switch (variable->Type)
                {
                  case ImmediateType_Tail:
                    stash_ptr = &stash_space->Tails[variable->Index];
                    array_ptr = &variables->Tails[variable->Index];
                    num_bits = 16;
                    break;
                  case ImmediateType_Hybrid:
                    stash_ptr = &stash_space->Hybrids[variable->Index];
                    array_ptr = &variables->Hybrids[variable->Index];
                    num_bits = 32;
                    break;
                }

                stash = *stash_ptr;

                if (stash == NULL)
                  complain(436, error_code_to_string(436), NULL, 0, 0);

                free_array(*array_ptr);
                *array_ptr = stash->Value;

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
        case StatementType_Remember:
        case StatementType_Abstain:
        case StatementType_Reinstate:
        case StatementType_GiveUp:
        case StatementType_WriteIn:
        case StatementType_ReadOut:
        case StatementType_ComeFrom:
          explode("THE INTERPRETER IS NOT YET FINISHED!");
      }

    if ((current_statement->This->SuckPointDestination != NULL)
     && (abstenance_list[Gerund_ComingFrom] == false))
      current_statement = current_statement->This->SuckPointDestination;
    else
      current_statement = current_statement->Next;
  }
}
