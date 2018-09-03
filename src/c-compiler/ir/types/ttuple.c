/** Handling for type tuple nodes
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"

// Create a new type tuple node
TTupleNode *newTTupleNode() {
	TTupleNode *tuple;
	newNode(tuple, TTupleNode, TTupleTag);
	tuple->vtype = voidType;
	tuple->types = newNodes(4);
	return tuple;
}

// Serialize a type tuple node
void ttuplePrint(TTupleNode *tuple) {
	INode **nodesp;
	uint32_t cnt;

	for (nodesFor(tuple->types, cnt, nodesp)) {
		inodePrintNode(*nodesp);
        if (cnt)
		    inodeFprint(",");
	}
}

// Check the type tuple node
void ttupleWalk(PassState *pstate, TTupleNode *tuple) {
	switch (pstate->pass) {
	case NameResolution:
		; break;
	case TypeCheck:
        ; break;
	}
}
