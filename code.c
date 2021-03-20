#include "globals.h"
#include "parse.h"
#include "symtab.h"
#include "code.h"

/* stack used for call */
TreeNode* paramStack[SIZE];
int top = 0;

/* stack routines*/
int pushParam(TreeNode* param)
{
    if(top == SIZE) 
      return 1;

    paramStack[top++] = param;
    return 0;
}

TreeNode* popParam()
{
  if(top == 0)
    return NULL;

  return paramStack[--top];
}

/* current location */
int emitLoc = 0 ;

/* Highest TM location emitted so far */
int highEmitLoc = 0;

/* skips "howMany" locations for later backpatch
 * returns the current code position
 */
int emitSkip( int howMany)
{  int i = emitLoc;
   emitLoc += howMany ;
   if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
   return i;
} 

/* backs up to a previously skipped location */
void emitBackup( int loc)
{ if (loc > highEmitLoc) emitComment("BUG in emitBackup");
  emitLoc = loc ;
} 

/* restores the current position to the highest previous position*/
void emitRestore(void)
{ emitLoc = highEmitLoc;}

/* prints a comment line */
void emitComment( char * c )
{ if (TraceCode) fprintf(code,"* %s\n",c);}

/* emits a register-only TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO( char *op, int r, int s, int t, char *c)
{ fprintf(code,"%3d:  %5s  %d,%d,%d ",emitLoc++,op,r,s,t);
  if (TraceCode) fprintf(code,"\t%s",c) ;
  fprintf(code,"\n") ;
  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} 

/* emits a register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM( char * op, int r, int d, int s, char *c)
{ fprintf(code,"%3d:  %5s  %d,%d(%d) ",emitLoc++,op,r,d,s);
  if (TraceCode) fprintf(code,"\t%s",c) ;
  fprintf(code,"\n") ;
  if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
} 

/*******************************/

/* generate standard prelude */
void emitPrelude()
{
   if (TraceCode) emitComment("Begin prelude");
   emitRM("LD",gp,0,zero,"load from location 0");
   emitRM("ST",zero,0,zero,"clear location 0");
   emitRM("LDA",sp,-(topTable()->size),gp,"allocate for global variables");
   if (TraceCode) emitComment("End of prelude");
}

/* generate codes for input() */
void emitInput()
{

   if (TraceCode) emitComment("Begin input()");
   FunSymbol* fun = lookup_fun("input");
   fun->offset = emitSkip(0);
   emitRO("IN",ax,0,0,"read input into ax");
   emitRM("LDA",sp,1,sp,"pop prepare");
   emitRM("LD",pc,-1,sp,"pop return addr");
   if (TraceCode) emitComment("End input()");
}

/* generate codes for output() */
void emitOutput()
{

   if (TraceCode) emitComment("Begin output()");
   FunSymbol* fun = lookup_fun("output");
   fun->offset = emitSkip(0);
   emitRM("LD",ax,1,sp,"load param into ax");
   emitRO("OUT",ax,0,0,"output using ax");
   emitRM("LDA",sp,1,sp,"pop prepare");
   emitRM("LD",pc,-1,sp,"pop return addr");
   if (TraceCode) emitComment("End output()");
}
  

/* emit one instruction to get the address of a var,
 * store the address in bx,
 * we can access the var by bx[0]
 */
void emitGetAddr(VarSymbol *var)
{

  switch(var->scope){
    case GLOBAL:
        if(var->type == TYPE_ARRAY){
          emitRM("LDA",bx,-(var->offset),gp,"get global array address");
        }
        else{
          emitRM("LDA",bx,-1-(var->offset),gp,"get global address");
        }
        break;
    case LOCAL:
        if(var->type == TYPE_ARRAY){
          emitRM("LDA",bx,-(var->offset),bp,"get local array address");
        }
        else{
          emitRM("LDA",bx,-1-(var->offset),bp,"get local address");
        }
        break;
    case PARAM:
        if(var->type == TYPE_ARRAY){
          emitRM("LD",bx,2+(var->offset),bp,"get param array address");
        }
        else{
          emitRM("LDA",bx,2+(var->offset),bp,"get param variable address");
        }
        break;
  }
}


/* emits 5 instructions to call a function,
 * before this we have pushed all parameters
 */
void emitCall(FunSymbol *fun)
{

  emitRM("LDA",ax,3,pc,"store returned PC");
  emitRM("LDA",sp,-1,sp,"push prepare");
  emitRM("ST",ax,0,sp,"push returned PC");
  emitRM("LDC",pc,fun->offset,0,"jump to function");
  emitRM("LDA",sp,fun->paramNum,sp,"release parameters");
}