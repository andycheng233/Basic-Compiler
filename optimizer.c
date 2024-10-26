#include "optimizer.h"

void Optimizer(NodeList *funcdecls)
{
     bool madeChange = true;

     while (madeChange)
     {
          if (!ConstantFolding(funcdecls) && !ConstProp(funcdecls) && !DeadAssign(funcdecls))
               madeChange = false;
     }
}