/*code generator
 * - To generate utimate TM codes from syntax tree and symbol tables.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "code.h"




/* recursively generates code by tree traversal */
void cGen( TreeNode * tree);

/* primary function
 * visit the syntax tree and generate codes
 */
void codeGen();


#endif