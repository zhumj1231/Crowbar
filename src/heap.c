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
crb_literal_to_crb_string_i(CRB_Interpreter *inter, CRB_Char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    ret->u.string.is_literal = CRB_TRUE;

    return ret;
}

CRB_Object *
CRB_literal_to_crb_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          CRB_Char *str)
{
    CRB_Object *ret;

    ret = crb_literal_to_crb_string_i(inter, str);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_create_crowbar_string_i(CRB_Interpreter *inter, CRB_Char *str)
{
    CRB_Object *ret;

    ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    inter->heap.current_heap_size += sizeof(CRB_Char) * (CRB_wcslen(str) + 1);
    ret->u.string.is_literal = CRB_FALSE;

    return ret;
}

CRB_Object *
CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                          CRB_Char *str)
{
    CRB_Object *ret;

    ret = crb_create_crowbar_string_i(inter, str);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Object *
crb_string_substr_i(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                    CRB_Object *str, int from, int len, int line_number)
{
    int org_len = CRB_wcslen(str->u.string.string);
    CRB_Char *new_str;

    if (from < 0 || from >= org_len) {
        crb_runtime_error(inter, env, line_number,
                          STRING_POS_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "len", org_len,
                          CRB_INT_MESSAGE_ARGUMENT, "pos", from,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    if (len < 0 || from + len > org_len) {
        crb_runtime_error(inter, env, line_number, STRING_SUBSTR_LEN_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "len", len,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    new_str = MEM_malloc(sizeof(CRB_Char) * (len+1));
    CRB_wcsncpy(new_str, str->u.string.string + from, len);
    new_str[len] = L'\0';

    return crb_create_crowbar_string_i(inter, new_str);
}

CRB_Object *
CRB_string_substr(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                  CRB_Object *str, int from, int len, int line_number)
{
    CRB_Object *ret;

    ret = crb_string_substr_i(inter, env, str, from, len, line_number);
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

void
CRB_array_resize(CRB_Interpreter *inter, CRB_Object *obj, int new_size)
{
    int new_alloc_size;
    CRB_Boolean need_realloc;
    int i;

    check_gc(inter);
    
    if (new_size > obj->u.array.alloc_size) {
        new_alloc_size = obj->u.array.alloc_size * 2;
        if (new_alloc_size < new_size) {
            new_alloc_size = new_size + ARRAY_ALLOC_SIZE;
        } else if (new_alloc_size - obj->u.array.alloc_size
                   > ARRAY_ALLOC_SIZE) {
            new_alloc_size = obj->u.array.alloc_size + ARRAY_ALLOC_SIZE;
        }
        need_realloc = CRB_TRUE;
    } else if (obj->u.array.alloc_size - new_size > ARRAY_ALLOC_SIZE) {
        new_alloc_size = new_size;
        need_realloc = CRB_TRUE;
    } else {
        need_realloc = CRB_FALSE;
    }
    if (need_realloc) {
        check_gc(inter);
        obj->u.array.array = MEM_realloc(obj->u.array.array,
                                         new_alloc_size * sizeof(CRB_Value));
        inter->heap.current_heap_size
            += (new_alloc_size - obj->u.array.alloc_size) * sizeof(CRB_Value);
        obj->u.array.alloc_size = new_alloc_size;
    }
    for (i = obj->u.array.size; i < new_size; i++) {
        obj->u.array.array[i].type = CRB_NULL_VALUE;
    }
    obj->u.array.size = new_size;
}

void
CRB_array_add(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value *v)
{
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));

    CRB_array_resize(inter, obj, obj->u.array.size + 1);
    obj->u.array.array[obj->u.array.size-1] = *v;
}

void
CRB_array_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_Object *obj, int pos,
                 CRB_Value *new_value, int line_number)
{
    int i;
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));

    if (pos < 0 || pos > obj->u.array.size) {
        crb_runtime_error(inter, env, line_number,
                          ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "size", obj->u.array.size,
                          CRB_INT_MESSAGE_ARGUMENT, "index", pos,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_resize(inter, obj, obj->u.array.size + 1);
    for (i = obj->u.array.size-2; i >= pos; i--) {
        obj->u.array.array[i+1] = obj->u.array.array[i];
    }
    obj->u.array.array[pos] = *new_value;
}

void
CRB_array_remove(CRB_Interpreter *inter, CRB_LocalEnvironment *env,
                 CRB_Object *obj, int pos, int line_number)
{
    int i;

    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    if (pos < 0 || pos > obj->u.array.size-1) {
        crb_runtime_error(inter, env, line_number,
                          ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
                          CRB_INT_MESSAGE_ARGUMENT, "size", obj->u.array.size,
                          CRB_INT_MESSAGE_ARGUMENT, "index", pos,
                          CRB_MESSAGE_ARGUMENT_END);
    }
    for (i = pos+1; i < obj->u.array.size; i++) {
        obj->u.array.array[i-1] = obj->u.array.array[i];
    }
    CRB_array_resize(inter, obj, obj->u.array.size - 1);
}

CRB_Object *
crb_create_assoc_i(CRB_Interpreter *inter)
{
    CRB_Object *ret;

    ret = alloc_object(inter, ASSOC_OBJECT);
    ret->u.assoc.member_count = 0;
    ret->u.assoc.member = NULL;

    return ret;
}

CRB_Object *
CRB_create_assoc(CRB_Interpreter *inter, CRB_LocalEnvironment *env)
{
    CRB_Object *ret;

    ret = crb_create_assoc_i(inter);
    add_ref_in_native_method(env, ret);

    return ret;
}

CRB_Value *
CRB_add_assoc_member(CRB_Interpreter *inter, CRB_Object *assoc,
                     char *name, CRB_Value *value, CRB_Boolean is_final)
{
    AssocMember *member_p;

    check_gc(inter);
    member_p = MEM_realloc(assoc->u.assoc.member,
                           sizeof(AssocMember)
                           * (assoc->u.assoc.member_count+1));
    member_p[assoc->u.assoc.member_count].name = name;
    member_p[assoc->u.assoc.member_count].value = *value;
    member_p[assoc->u.assoc.member_count].is_final = is_final;
    assoc->u.assoc.member = member_p;
    assoc->u.assoc.member_count++;
    inter->heap.current_heap_size += sizeof(AssocMember);

    return &member_p[assoc->u.assoc.member_count-1].value;
}

void
CRB_add_assoc_member2(CRB_Interpreter *inter, CRB_Object *assoc,
                      char *name, CRB_Value *value)
{
    int i;

    if (assoc->u.assoc.member_count > 0) {
        for (i = 0; i < assoc->u.assoc.member_count; i++) {
            if (!strcmp(assoc->u.assoc.member[i].name, name)) {
                break;
            }
        }
        if (i < assoc->u.assoc.member_count) {
            /* BUGBUG */
            assoc->u.assoc.member[i].value = *value;
            return;
        }
    }
    CRB_add_assoc_member(inter, assoc, name, value, CRB_FALSE);
}

CRB_Value *
CRB_search_assoc_member(CRB_Object *assoc, char *member_name)
{
    CRB_Value *ret = NULL;
    int i;

    if (assoc->u.assoc.member_count == 0)
        return NULL;

    for (i = 0; i < assoc->u.assoc.member_count; i++) {
        if (!strcmp(assoc->u.assoc.member[i].name, member_name)) {
            ret = &assoc->u.assoc.member[i].value;
            break;
        }
    }

    return ret;
}

CRB_Value *
CRB_search_assoc_member_w(CRB_Object *assoc, char *member_name,
                          CRB_Boolean *is_final)
{
    CRB_Value *ret = NULL;
    int i;

    if (assoc->u.assoc.member_count == 0)
        return NULL;

    for (i = 0; i < assoc->u.assoc.member_count; i++) {
        if (!strcmp(assoc->u.assoc.member[i].name, member_name)) {
            ret = &assoc->u.assoc.member[i].value;
            *is_final = assoc->u.assoc.member[i].is_final;
            break;
        }
    }
    return ret;
}

static void
gc_mark(CRB_Object *obj)
{
    if (obj->marked)
        return;

    obj->marked = CRB_TRUE;

    if (obj->type == ARRAY_OBJECT) {
        int i;
        for (i = 0; i < obj->u.array.size; i++) {
            if (dkc_is_object_value(obj->u.array.array[i].type)) {
                gc_mark(obj->u.array.array[i].u.object);
            }
        }
    }
}

static void
gc_reset_mark(CRB_Object *obj)
{
    obj->marked = CRB_FALSE;
}

static void
gc_mark_ref_in_native_method(CRB_LocalEnvironment *env)
{
    RefInNativeFunc *ref;

    for (ref = env->ref_in_native_method; ref; ref = ref->next) {
        gc_mark(ref->object);
    }
}

static void
gc_mark_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    Variable *v;
    CRB_LocalEnvironment *lv;
    int i;

    for (obj = inter->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }
    
    for (v = inter->variable; v; v = v->next) {
        if (dkc_is_object_value(v->value.type)) {
            gc_mark(v->value.u.object);
        }
    }
    
    for (lv = inter->top_environment; lv; lv = lv->next) {
        for (v = lv->variable; v; v = v->next) {
            if (dkc_is_object_value(v->value.type)) {
                gc_mark(v->value.u.object);
            }
        }
        gc_mark_ref_in_native_method(lv);
    }

    for (i = 0; i < inter->stack.stack_pointer; i++) {
        if (dkc_is_object_value(inter->stack.stack[i].type)) {
            gc_mark(inter->stack.stack[i].u.object);
        }
    }
}

static void
gc_dispose_object(CRB_Interpreter *inter, CRB_Object *obj)
{
    switch (obj->type) {
    case ARRAY_OBJECT:
        inter->heap.current_heap_size
            -= sizeof(CRB_Value) * obj->u.array.alloc_size;
        MEM_free(obj->u.array.array);
        break;
    case STRING_OBJECT:
        if (!obj->u.string.is_literal) {
            inter->heap.current_heap_size
                -= sizeof(CRB_Char) * (CRB_wcslen(obj->u.string.string) + 1);
            MEM_free(obj->u.string.string);
        }
        break;
    case OBJECT_TYPE_COUNT_PLUS_1:
    default:
        DBG_assert(0, ("bad type..%d\n", obj->type));
    }
    inter->heap.current_heap_size -= sizeof(CRB_Object);
    MEM_free(obj);
}

static void
gc_sweep_objects(CRB_Interpreter *inter)
{
    CRB_Object *obj;
    CRB_Object *tmp;

    for (obj = inter->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                inter->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
            gc_dispose_object(inter, obj);
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
}

void
crb_garbage_collect(CRB_Interpreter *inter)
{
    gc_mark_objects(inter);
    gc_sweep_objects(inter);
}