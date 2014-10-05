#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"

static void
check_gc(CRB_Interpreter *inter)
{
#if 0
    crb_garbage_collect(inter);
#endif
    if (inter->heap.current_heap_size > inter->heap.current_threshold) {
        /* fprintf(stderr, "garbage collecting..."); */
        crb_garbage_collect(inter);
        /* fprintf(stderr, "done.\n"); */

        inter->heap.current_threshold
            = inter->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static CRB_Object *
alloc_object(CRB_Interpreter *inter, ObjectType type)
{
    CRB_Object *ret;

    check_gc(inter);
    ret = MEM_malloc(sizeof(CRB_Object));
    inter->heap.current_heap_size += sizeof(CRB_Object);
    ret->type = type;
    ret->marked = CRB_FALSE;
    ret->prev = NULL;
    ret->next = inter->heap.header;
    inter->heap.header = ret;
    if (ret->next) {
        ret->next->prev = ret;
    }

    return ret;
}

static void
add_ref_in_native_method(CRB_LocalEnvironment *env, CRB_Object *obj)
{
    RefInNativeFunc *new_ref;

    new_ref = MEM_malloc(sizeof(RefInNativeFunc));
    new_ref->object = obj;
    new_ref->next = env->ref_in_native_method;
    env->ref_in_native_method = new_ref;
}

CRB_Object *
crb_literal_to_crb_string(CRB_Interpreter *inter, char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    ret->u.string.is_literal = CRB_TRUE;

    return ret;
}

CRB_Object *
crb_create_crowbar_string_i(CRB_Interpreter *inter, char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    inter->heap.current_heap_size += strlen(str) + 1;
    ret->u.string.is_literal = CRB_FALSE;

    return ret;
}

CRB_Object *
CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          char *str)
{
    CRB_Object *ret;

    ret = crb_create_crowbar_string_i(inter, str);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_create_array_i(CRB_Interpreter *inter, int size)
{
    CRB_Object *ret;

    ret = alloc_object(inter, ARRAY_OBJECT);
    ret->u.array.size = size;
    ret->u.array.alloc_size = size;
    ret->u.array.array = MEM_malloc(sizeof(CRB_Value) * size);
    inter->heap.current_heap_size += sizeof(CRB_Value) * size;

    return ret;
}

CRB_Object *
CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 int size)
{
    CRB_Object *ret;

    ret = crb_create_array_i(inter, size);
    add_ref_in_native_method(env, ret);

    return ret;
}