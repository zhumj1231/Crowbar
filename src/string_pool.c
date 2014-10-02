#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_String *
alloc_crb_string(CRB_Interpreter *inter, char *str, CRB_Boolean is_literal)
{
    CRB_String *ret;

    ret = MEM_malloc(sizeof(CRB_String));
    ret->ref_count = 0;
    ret->is_literal = is_literal;
    ret->string = str;

    return ret;
}