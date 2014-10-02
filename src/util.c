#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static CRB_Interpreter *st_current_interpreter;

CRB_Interpreter *
crb_get_current_interpreter(void)
{
    return st_current_interpreter;
}

void
crb_set_current_interpreter(CRB_Interpreter *inter)
{
    st_current_interpreter = inter;
}
