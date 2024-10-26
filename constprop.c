/*
********************************************************************************
  CONSTPROP.C : IMPLEMENT THE DOWNSTREAM CONSTANT PROPOGATION OPTIMIZATION HERE
*********************************************************************************
*/

#include "constprop.h"

refConst *lastNode, *headNode;
/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO FREE THE CONSTANTS-ASSOCIATED VARIABLES LIST
************************************************************************
*/
void FreeConstList()
{
  refConst *tmp;
  while (headNode != NULL)
  {
    tmp = headNode;
    headNode = headNode->next;
    free(tmp);
  }
}

/*
*************************************************************************
  FUNCTION TO ADD A CONSTANT VALUE AND THE ASSOCIATED VARIABLE TO THE LIST
**************************************************************************
*/
void UpdateConstList(char *name, long val)
{
  refConst *node = malloc(sizeof(refConst));
  if (node == NULL)
    return;
  node->name = name;
  node->val = val;
  node->next = NULL;
  if (headNode == NULL)
  {
    lastNode = node;
    headNode = node;
  }
  else
  {
    lastNode->next = node;
    lastNode = node;
  }
}

/*
*****************************************************************************
  FUNCTION TO LOOKUP IF A CONSTANT ASSOCIATED VARIABLE IS ALREADY IN THE LIST
******************************************************************************
*/
refConst *LookupConstList(char *name)
{
  refConst *node;
  node = headNode;
  while (node != NULL)
  {
    if (!strcmp(name, node->name))
      return node;
    node = node->next;
  }
  return NULL;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS BELOW THIS LINE
**********************************************************************************************************************************
*/

/*
************************************************************************************
  THIS FUNCTION IS MEANT TO UPDATE THE CONSTANT LIST WITH THE ASSOCIATED VARIABLE
  AND CONSTANT VALUE WHEN ONE IS SEEN. IT SHOULD ALSO PROPOGATE THE CONSTANTS WHEN
  WHEN APPLICABLE. YOU CAN ADD A NEW FUNCTION IF YOU WISH TO MODULARIZE BETTER.
*************************************************************************************
*/
void TrackConst(NodeList *statements)
{
  Node *node;
  while (statements != NULL)
  {
    node = statements->node;
    Node *expression;
    refConst *c;

    if (node->stmtCode == ASSIGN)
      expression = node->right;
    else if (node->stmtCode == RETURN)
      expression = node->left;

    if (expression->exprCode == CONSTANT)
      UpdateConstList(node->name, expression->value);

    else if (expression->exprCode == VARIABLE || expression->exprCode == PARAMETER)
    {
      c = LookupConstList(expression->name);
      if (c != NULL)
      {
        expression->left = NULL;
        expression->exprCode = CONSTANT;
        free(expression->name);
        expression->name = NULL;
        expression->value = c->val;
      }
    }

    else if (expression->exprCode == OPERATION)
    {
      if (expression->opCode == NEGATE)
      {
        c = LookupConstList(expression->left->name);
        if (c != NULL)
        {
          expression->exprCode = CONSTANT;
          expression->opCode = O_NONE;
          free(expression->name);
          expression->name = NULL;
          expression->value = -c->val;
          FreeVariable(expression->left);
          expression->left = NULL;
        }
      }

      else if (expression->opCode == FUNCTIONCALL)
      {
        NodeList *arg = expression->arguments;
        while (arg != NULL)
        {
          c = LookupConstList(arg->node->name);
          if (c != NULL)
          {
            arg->node->exprCode = CONSTANT;
            arg->node->opCode = O_NONE;
            free(arg->node->name);
            arg->node->name = NULL;
            arg->node->value = c->val;
          }

          arg = arg->next;
        }
      }

      else
      {
        if (expression->left->exprCode == VARIABLE || expression->left->exprCode == PARAMETER)
        {
          c = LookupConstList(expression->left->name);
          if (c != NULL)
          {
            expression->left->left = NULL;
            expression->left->exprCode = CONSTANT;
            expression->left->opCode = O_NONE;
            free(expression->left->name);
            expression->left->name = NULL;
            expression->left->value = c->val;
          }
        }

        if (expression->right->exprCode == VARIABLE || expression->right->exprCode == PARAMETER)
        {
          c = LookupConstList(expression->right->name);
          if (c != NULL)
          {
            expression->right->left = NULL;
            expression->right->exprCode = CONSTANT;
            expression->right->opCode = O_NONE;
            free(expression->right->name);
            expression->right->name = NULL;
            expression->right->value = c->val;
          }
        }
      }
    }

    statements = statements->next;
  }
}

bool ConstProp(NodeList *worklist)
{
  while (worklist != NULL)
  {
    TrackConst(worklist->node->statements);
    worklist = worklist->next;
    FreeConstList();
  }

  return madeChange;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE
**********************************************************************************************************************************
*/
