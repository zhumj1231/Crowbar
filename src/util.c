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

FunctionDefinition *
crb_search_function(char *name)
{
    FunctionDefinition *pos;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    for (pos = inter->function_list; pos; pos = pos->next) {
        if (!strcmp(pos->name, name))
            break;
    }
    return pos;
}

void *
crb_malloc(size_t size)
{
    void *p;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    p = MEM_storage_malloc(inter->interpreter_storage, size);

    return p;
}

void *
crb_execute_malloc(CRB_Interpreter *inter, size_t size)
{
    void *p;

    p = MEM_storage_malloc(inter->execute_storage, size);

    return p;
}

void *
crb_malloc(size_t size)
{
    void *p;
    CRB_Interpreter *inter;

    inter = crb_get_current_interpreter();
    p = MEM_storage_malloc(inter->interpreter_storage, size);

    return p;
}

void *
crb_execute_malloc(CRB_Interpreter *inter, size_t size)
{
    void *p;

    p = MEM_storage_malloc(inter->execute_storage, size);

    return p;
}
