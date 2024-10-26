/*
***********************************************************************
  CODEGEN.C : IMPLEMENT CODE GENERATION HERE
************************************************************************
*/
#include "codegen.h"

int argCounter;
int lastUsedOffset;
char lastOffsetUsed[100];
FILE *fptr;
regInfo *regList, *regHead, *regLast;
varStoreInfo *varList, *varHead, *varLast;

/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO INITIALIZE THE ASSEMBLY FILE WITH FUNCTION DETAILS
************************************************************************
*/
void InitAsm(char *funcName)
{
    fprintf(fptr, "\n.globl %s", funcName);
    fprintf(fptr, "\n%s:", funcName);

    // Init stack and base ptr
    fprintf(fptr, "\npushq %%rbp");
    fprintf(fptr, "\nmovq %%rsp, %%rbp");
}

/*
***************************************************************************
   FUNCTION TO WRITE THE RETURNING CODE OF A FUNCTION IN THE ASSEMBLY FILE
****************************************************************************
*/
void RetAsm()
{
    fprintf(fptr, "\npopq %%rbp");
    fprintf(fptr, "\nretq\n");
}

/*
***************************************************************************
  FUNCTION TO CONVERT OFFSET FROM LONG TO CHAR STRING
****************************************************************************
*/
void LongToCharOffset()
{
    lastUsedOffset = lastUsedOffset - 8;
    snprintf(lastOffsetUsed, 100, "%d", lastUsedOffset);
    strcat(lastOffsetUsed, "(%rbp)");
}

/*
***************************************************************************
  FUNCTION TO CONVERT CONSTANT VALUE TO CHAR STRING
****************************************************************************
*/
void ProcessConstant(Node *opNode)
{
    char value[10];
    LongToCharOffset();
    snprintf(value, 10, "%ld", opNode->value);
    char str[100];
    snprintf(str, 100, "%d", lastUsedOffset);
    strcat(str, "(%rbp)");
    AddVarInfo("", str, opNode->value, true);
    fprintf(fptr, "\nmovq  $%s, %s", value, str);
}

/*
***************************************************************************
  FUNCTION TO SAVE VALUE IN ACCUMULATOR (RAX)
****************************************************************************
*/
void SaveValInRax(char *name)
{
    char *tempReg;
    tempReg = GetNextAvailReg(true);
    if (!(strcmp(tempReg, "NoReg")))
    {
        LongToCharOffset();
        fprintf(fptr, "\n movq %%rax, %s", lastOffsetUsed);
        UpdateVarInfo(name, lastOffsetUsed, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
    else
    {
        fprintf(fptr, "\nmovq %%rax, %s", tempReg);
        UpdateRegInfo(tempReg, 0);
        UpdateVarInfo(name, tempReg, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
}

/*
***********************************************************************
  FUNCTION TO ADD VARIABLE INFORMATION TO THE VARIABLE INFO LIST
************************************************************************
*/
void AddVarInfo(char *varName, char *location, long val, bool isConst)
{
    varStoreInfo *node = malloc(sizeof(varStoreInfo));
    node->varName = varName;
    node->value = val;
    strcpy(node->location, location);
    node->isConst = isConst;
    node->next = NULL;
    node->prev = varLast;
    if (varHead == NULL)
    {
        varHead = node;
        varLast = node;
        varList = node;
    }
    else
    {
        // node->prev = varLast;
        varLast->next = node;
        varLast = varLast->next;
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO FREE THE VARIABLE INFORMATION LIST
************************************************************************
*/
void FreeVarList()
{
    varStoreInfo *tmp;
    while (varHead != NULL)
    {
        tmp = varHead;
        varHead = varHead->next;
        free(tmp);
    }
}

/*
***********************************************************************
  FUNCTION TO LOOKUP VARIABLE INFORMATION FROM THE VARINFO LIST
************************************************************************
*/
char *LookUpVarInfo(char *name, long val)
{
    varList = varLast;
    if (varList == NULL)
        printf("NULL varlist");
    while (varList != NULL)
    {
        if (varList->isConst == true)
        {
            if (varList->value == val)
                return varList->location;
        }
        else
        {
            if (!strcmp(name, varList->varName))
                return varList->location;
        }
        varList = varList->prev;
    }
    varList = varHead;
    return "";
}

/*
***********************************************************************
  FUNCTION TO UPDATE VARIABLE INFORMATION
************************************************************************
*/
void UpdateVarInfo(char *varName, char *location, long val, bool isConst)
{

    if (!(strcmp(LookUpVarInfo(varName, val), "")))
    {
        AddVarInfo(varName, location, val, isConst);
    }
    else
    {
        varList = varHead;
        if (varList == NULL)
            printf("NULL varlist");
        while (varList != NULL)
        {
            if (!strcmp(varList->varName, varName))
            {
                varList->value = val;
                strcpy(varList->location, location);
                varList->isConst = isConst;
                break;
            }
            varList = varList->next;
        }
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE VARIABLE INFORMATION LIST
************************************************************************
*/
void PrintVarListInfo()
{
    varList = varHead;
    if (varList == NULL)
        printf("NULL varlist");
    while (varList != NULL)
    {
        if (!varList->isConst)
        {
            printf("\t %s : %s", varList->varName, varList->location);
        }
        else
        {
            printf("\t %ld : %s", varList->value, varList->location);
        }
        varList = varList->next;
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO ADD NEW REGISTER INFORMATION TO THE REGISTER INFO LIST
************************************************************************
*/
void AddRegInfo(char *name, int avail)
{

    regInfo *node = malloc(sizeof(regInfo));
    node->regName = name;
    node->avail = avail;
    node->next = NULL;

    if (regHead == NULL)
    {
        regHead = node;
        regList = node;
        regLast = node;
    }
    else
    {
        regLast->next = node;
        regLast = node;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO FREE REGISTER INFORMATION LIST
************************************************************************
*/
void FreeRegList()
{
    regInfo *tmp;
    while (regHead != NULL)
    {
        tmp = regHead;
        regHead = regHead->next;
        free(tmp);
    }
}

/*
***********************************************************************
  FUNCTION TO UPDATE THE AVAILIBILITY OF REGISTERS IN THE REG INFO LIST
************************************************************************
*/
void UpdateRegInfo(char *regName, int avail)
{
    while (regList != NULL)
    {
        if (regName == regList->regName)
        {
            regList->avail = avail;
        }
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO RETURN THE NEXT AVAILABLE REGISTER
************************************************************************
*/
char *GetNextAvailReg(bool noAcc)
{
    regList = regHead;
    if (regList == NULL)
        printf("NULL reglist");
    while (regList != NULL)
    {
        if (regList->avail == 1)
        {
            if (!noAcc)
                return regList->regName;
            // if not rax and dont return accumulator set to true, return the other reg
            // if rax and noAcc == true, skip to next avail
            if (noAcc && strcmp(regList->regName, "%rax"))
            {
                return regList->regName;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return "NoReg";
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF ANY REGISTER APART FROM OR INCLUDING
  THE ACCUMULATOR(RAX) IS AVAILABLE
************************************************************************
*/
int IfAvailReg(bool noAcc)
{
    regList = regHead;
    if (regList == NULL)
        printf("NULL reglist");
    while (regList != NULL)
    {
        if (regList->avail == 1)
        {
            // registers available
            if (!noAcc)
                return 1;
            if (noAcc && strcmp(regList->regName, "%rax"))
            {
                return 1;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return 0;
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF A SPECIFIC REGISTER IS AVAILABLE
************************************************************************
*/
bool IsAvailReg(char *name)
{
    regList = regHead;
    if (regList == NULL)
        printf("NULL reglist");
    while (regList != NULL)
    {
        if (!strcmp(regList->regName, name))
        {
            if (regList->avail == 1)
            {
                return true;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return false;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE REGISTER INFORMATION
************************************************************************
*/
void PrintRegListInfo()
{
    regList = regHead;
    if (regList == NULL)
        printf("NULL reglist");
    while (regList != NULL)
    {
        printf("\t %s : %d", regList->regName, regList->avail);
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO CREATE THE REGISTER LIST
************************************************************************
*/
void CreateRegList()
{
    // Create the initial reglist which can be used to store variables.
    // 4 general purpose registers : AX, BX, CX, DX
    // 4 special purpose : SP, BP, SI , DI.
    // Other registers: r8, r9
    // You need to decide which registers you will add in the register list
    // use. Can you use all of the above registers?

    AddRegInfo("rdi", 1);
    AddRegInfo("rsi", 1);
    AddRegInfo("rdx", 1);
    AddRegInfo("rcx", 1);
    AddRegInfo("r8", 1);
    AddRegInfo("r9", 1);
}

/*
***********************************************************************
  THIS FUNCTION IS MEANT TO PUT THE FUNCTION ARGUMENTS FROM STACK
************************************************************************
*/
int PutArgumentsFromStack(NodeList *arguments)
{
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    while (arguments != NULL)
    {
        /*
         ***********************************************************************
                  TODO : YOUR CODE HERE
          THINK ABOUT WHERE EACH ARGUMENT COMES FROM. EXAMPLE WHERE IS THE
          FIRST ARGUMENT OF A FUNCTION STORED.
         ************************************************************************
         */
        arguments = arguments->next;
    }
    return argCounter;
}

/*
*************************************************************************
  THIS FUNCTION IS MEANT TO GET THE FUNCTION ARGUMENTS ON THE  STACK
**************************************************************************
*/
void PutArgumentsOnStack(NodeList *arguments)
{
    argCounter = 0;
    regInfo *head = regHead;
    while (arguments != NULL)
    {
        argCounter++;
        LongToCharOffset();
        AddVarInfo(arguments->node->name, lastOffsetUsed, 0, false);
        fprintf(fptr, "\nmovq %%%s, %d(%%rbp)", head->regName, lastUsedOffset);
        head = head->next;
        arguments = arguments->next;
    }
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO PROCESS EACH CODE STATEMENT AND GENERATE
  ASSEMBLY FOR IT.
  TIP: YOU CAN MODULARIZE BETTER AND ADD NEW SMALLER FUNCTIONS IF YOU
  WANT THAT CAN BE CALLED FROM HERE.
 ************************************************************************
 */
void ProcessStatements(NodeList *statements)
{
    spaceForAssignments(statements);
    while (statements != NULL)
    {
        Node *node = statements->node;
        if (node->type == STATEMENT && node->stmtCode == ASSIGN)
        {
            // should never happen due to optimization but just in case
            if (node->right->exprCode == CONSTANT)
            {
                LongToCharOffset();
                AddVarInfo(node->name, lastOffsetUsed, node->right->value, false);
                fprintf(fptr, "\nmovq $%ld, %d(%%rbp)", node->right->value, lastUsedOffset);
            }

            else if (node->right->exprCode == VARIABLE || node->right->exprCode == PARAMETER)
            {
                LongToCharOffset();
                AddVarInfo(node->name, lastOffsetUsed, 0, false);
                fprintf(fptr, "\nmovq %s, %%rcx", LookUpVarInfo(node->right->name, 0));
                fprintf(fptr, "\nmovq %%rcx, %d(%%rbp)", lastUsedOffset);
            }

            else if (node->right->exprCode == OPERATION)
            {
                LongToCharOffset();
                AddVarInfo(node->name, lastOffsetUsed, 0, false);

                if (node->right->opCode == FUNCTIONCALL)
                {
                    NodeList *args = node->right->arguments;
                    regInfo *head = regHead;
                    while (args != NULL)
                    {
                        if (args->node->exprCode == CONSTANT)
                            fprintf(fptr, "\nmovq $%ld, %%%s", args->node->value, head->regName);
                        else if (args->node->exprCode == VARIABLE || args->node->exprCode == PARAMETER)
                        {
                            fprintf(fptr, "\nmovq %s, %%%s", LookUpVarInfo(args->node->name, 0), head->regName);
                        }

                        head = head->next;
                        args = args->next;
                    }

                    fprintf(fptr, "\ncallq %s", node->right->left->name);
                    fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                }

                // fix division, negate, and bitshifts
                // calcexpresValue
                else if (node->right->opCode == NEGATE)
                {
                    fprintf(fptr, "\nmovq %s, %%rcx", LookUpVarInfo(node->right->left->name, 0));
                    fprintf(fptr, "\nnegq %%rcx");
                    fprintf(fptr, "\nmovq %%rcx, %s", LookUpVarInfo(node->name, 0));
                }

                else
                {
                    char *left = malloc(100 * sizeof(char));
                    char *right = malloc(100 * sizeof(char));

                    if (node->right->left->exprCode == CONSTANT)
                        sprintf(left, "$%ld", node->right->left->value);

                    else if (node->right->left->exprCode == VARIABLE || node->right->left->exprCode == PARAMETER)
                        sprintf(left, "%s", LookUpVarInfo(node->right->left->name, 0));

                    if (node->right->right->exprCode == CONSTANT)
                        sprintf(right, "$%ld", node->right->right->value);

                    else if (node->right->right->exprCode == VARIABLE || node->right->right->exprCode == PARAMETER)
                        sprintf(right, "%s", LookUpVarInfo(node->right->right->name, 0));

                    fprintf(fptr, "\nmovq %s, %%rax", left);
                    fprintf(fptr, "\nmovq %s, %%rcx", right);

                    switch (node->right->opCode)
                    {

                    case MULTIPLY:
                        fprintf(fptr, "\nimulq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case DIVIDE:
                        fprintf(fptr, "\ncqto");
                        fprintf(fptr, "\nidivq %%rcx");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case ADD:
                        fprintf(fptr, "\naddq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case SUBTRACT:
                        fprintf(fptr, "\nsubq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case BOR:
                        fprintf(fptr, "\norq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case BAND:
                        fprintf(fptr, "\nandq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case BXOR:
                        fprintf(fptr, "\nxorq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case BSHR:
                        fprintf(fptr, "\nsarq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    case BSHL:
                        fprintf(fptr, "\nsalq %%rcx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(node->name, 0));
                        break;
                    default:
                        fprintf(stderr, "New Operation Case Somehow Found");
                        break;
                    }

                    free(left);
                    free(right);
                }
            }
        }

        else if (node->type == STATEMENT && node->stmtCode == RETURN)
        {
            if (node->left->exprCode == CONSTANT)
                fprintf(fptr, "\nmovq $%ld, %%rax", node->left->value);

            if (node->left->exprCode == VARIABLE || node->left->exprCode == PARAMETER)
                fprintf(fptr, "\nmovq %s, %%rax", LookUpVarInfo(node->left->name, node->left->value));
        }

        statements = statements->next;
    }
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO DO CODEGEN FOR ALL THE FUNCTIONS IN THE FILE
 ************************************************************************
*/
void Codegen(NodeList *worklist)
{
    fptr = fopen("assembly.s", "w+");

    if (fptr == NULL)
    {
        printf("\n Could not create assembly file");
        return;
    }

    CreateRegList();

    while (worklist != NULL)
    {
        Optimizer(worklist);
        InitAsm(worklist->node->name);
        PutArgumentsOnStack(worklist->node->arguments);
        ProcessStatements(worklist->node->statements);
        fprintf(fptr, "\nmovq %%rbp, %%rsp");
        RetAsm();
        FreeVarList();

        worklist = worklist->next;
    }

    FreeRegList();
    lastUsedOffset = 0;

    fclose(fptr);
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS BELOW THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/

void spaceForAssignments(NodeList* statements)
{
    int assignCount = 0;
    while(statements != NULL)
    {
        if(statements->node->type == STATEMENT)
            assignCount++;
        statements = statements->next;
    }

    fprintf(fptr, "\nsubq $%d, %%rsp", (argCounter+assignCount)*8);
}
/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/
