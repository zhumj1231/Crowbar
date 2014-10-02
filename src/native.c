#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "CRB_dev.h"
#include "crowbar.h"

#define NATIVE_LIB_NAME "crowbar.lang.file"

static CRB_NativePointerInfo st_native_lib_info = {
    NATIVE_LIB_NAME
};

CRB_Value crb_nv_print_proc(CRB_Interpreter *interpreter,
                            int arg_count, CRB_Value *args)
{
    CRB_Value value;

    value.type = CRB_NULL_VALUE;

    if (arg_count < 1) {
        crb_runtime_error(0, ARGUMENT_TOO_FEW_ERR,
                          MESSAGE_ARGUMENT_END);
    } else if (arg_count > 1) {
        crb_runtime_error(0, ARGUMENT_TOO_MANY_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    switch (args[0].type) {
    case CRB_BOOLEAN_VALUE:
        if (args[0].u.boolean_value) {
            printf("true");
        } else {
            printf("false");
        }
        break;
    case CRB_INT_VALUE:
        printf("%d", args[0].u.int_value);
        break;
    case CRB_DOUBLE_VALUE:
        printf("%f", args[0].u.double_value);
        break;
    case CRB_STRING_VALUE:
        printf("%s", args[0].u.string_value->string);
        break;
    case CRB_NATIVE_POINTER_VALUE:
        printf("(%s:%p)",
               args[0].u.native_pointer.info->name,
               args[0].u.native_pointer.pointer);
        break;
    case CRB_NULL_VALUE:
        printf("null");
        break;
    }

    return value;
}

