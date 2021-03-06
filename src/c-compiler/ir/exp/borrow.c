/** Handling for borrow expression nodes
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"

#include <assert.h>

// Inject a typed, borrowed node on some node (expected to be an lval)
void borrowMutRef(INode **nodep, INode* type, INode *perm) {
    INode *node = *nodep;
    // Rather than borrow from a deref, just return the ptr node we are de-reffing
    if (node->tag == DerefTag) {
        StarNode *derefnode = (StarNode *)node;
        *nodep = derefnode->vtexp;
        return;
    }
  
    if (iexpIsLval(node) == 0) {
        errorMsgNode(node, ErrorInvType, "Auto-borrowing can only be done on an lval");
    }
    RefNode *reftype = newRefNodeFull(RefTag, node, borrowRef, perm, type);
    RefNode *borrownode = newRefNodeFull(BorrowTag, node, borrowRef, perm, node);
    borrownode->vtype = (INode*)reftype;
    *nodep = (INode*)borrownode;
}

// Serialize borrow node
void borrowPrint(RefNode *node) {
    inodeFprint("&(");
    inodePrintNode(node->vtype);
    inodeFprint("->");
    inodePrintNode(node->vtexp);
    inodeFprint(")");
}

// Analyze borrow node
void borrowTypeCheck(TypeCheckState *pstate, RefNode **nodep) {
    RefNode *node = *nodep;
    if (iexpTypeCheckAny(pstate, &node->vtexp) == 0 || iexpIsLval(node->vtexp) == 0)
        return;

    // Auto-deref the exp, if we are borrowing a reference to a reference's field or indexed value
    INode *exptype = iexpGetTypeDcl(node->vtexp);
    if ((node->flags & FlagSuffix) && (exptype->tag == RefTag || exptype->tag == PtrTag || exptype->tag == ArrayRefTag)) {
        StarNode *deref = newStarNode(DerefTag);
        deref->vtexp = node->vtexp;
        if (exptype->tag == ArrayRefTag)
            deref->vtype = (INode*)newArrayDerefNodeFrom((RefNode*)exptype);
        else
            deref->vtype = ((RefNode*)exptype)->vtexp;  // assumes StarNode has field in same place
        node->vtexp = (INode*)deref;
    }

    // Setup lval, perm and scope info as if we were borrowing from a global constant literal.
    // If not, extract this info from expression nodes
    uint16_t scope = 0;  // global
    INode *lval = node->vtexp;
    INode *lvalperm = (INode*)immPerm;
    scope = 0;  // Global
    if (lval->tag != StringLitTag) {
        // lval is the variable or variable sub-structure we want to get a reference to
        // From it, obtain variable we are borrowing from and actual/calculated permission
        INode *lvalvar = iexpGetLvalInfo(lval, &lvalperm, &scope);
        if (lvalvar == NULL) {
            node->vtype = (INode*)newRefNodeFull(RefTag, (INode*)node, node->region, node->perm, (INode*)unknownType); // To avoid a crash later
            return;
        }
        // Set lifetime of reference to borrowed variable's lifetime
        if (lvalvar->tag == VarDclTag)
            scope = ((VarDclNode*)lvalvar)->scope;
    }
    INode *lvaltype = ((IExpNode*)lval)->vtype;

    // The reference's value type is currently unknown (NULL).
    // Let's infer this value type from the lval we are borrowing from
    uint16_t tag = RefTag;
    INode *refvtype;
    if (lvaltype->tag == ArrayTag) {
        // Borrowing from a fixed size array creates an array reference
        tag = ArrayRefTag;
        refvtype = arrayElemType(lvaltype);
    }
    else if (lvaltype->tag == ArrayDerefTag) {
        tag = ArrayRefTag;
        refvtype = ((RefNode*)lvaltype)->vtexp;
    }
    else
        refvtype = lvaltype;

    // Ensure requested/inferred permission matches lval's permission
    INode *refperm = node->perm;
    if (refperm == unknownType)
        refperm = newPermUseNode(itypeIsConcrete(refvtype) ? constPerm : opaqPerm);
    if (!permMatches(refperm, lvalperm))
        errorMsgNode((INode *)node, ErrorBadPerm, "Borrowed reference cannot obtain this permission");

    RefNode *reftype = newRefNodeFull(tag, (INode*)node, borrowRef, refperm, refvtype);
    reftype->scope = scope;
    node->vtype = (INode *)reftype;
}

// Perform data flow analysis on addr node
void borrowFlow(FlowState *fstate, RefNode **nodep) {
    RefNode *node = *nodep;
    RefNode *reftype = (RefNode *)node->vtype;
    // Borrowed reference:  Deactivate source variable if necessary
}
