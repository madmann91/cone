/** Handling for break nodes
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"
#include "../../shared/memory.h"
#include "../../parser/lexer.h"
#include "../nametbl.h"
#include "../../shared/error.h"

// Semantic pass on break or continue
void breakPass(PassState *pstate, INode *node) {
	if (pstate->pass==NameResolution && !(pstate->flags & PassWithinWhile))
		errorMsgNode(node, ErrorNoWhile, "break/continue may only be used within a while/each block");
}