#include <string.h>
#include "crowbar.h"

MessageFormat crb_compile_error_message_format[] = {
    {"dummy"},
    {"Syntex error : ($(token))"},
    {"Bad char : ($(bad_char))"},
    {"Duplicate function names : ($(name))"},
    {"dummy"},
};

MessageFormat crb_runtime_error_message_format[] = {
    {"dummy"},
    {"Can not find the variable : ($(name))."},
    {"Can not find the function : ($(name))."},
    {"Too many arguments for the function."},
    {"Too few arguments for the function."},
    {"The type of the value of the condition statement must be boolean."},
    {"The types of the operands of the subtraction must be numeric."},
    {"The types of the operands of the operator : $(operator) are incorrect."},
    {"The types of the operands of the operator : $(operator) can not be boolean."},
    {"The function of fopen() should get two strings of the path and the method of opening as arguments."},
    {"The function of fclose() should get one pointer of file as argument."},
    {"The function of fgets() should get one pointer of file as argument."},
    {"The function of fputs() should get one pointer of file and one string as arguments."},
    {"The null can only be used for the operators == and != , the operator $(operator) is illegal."},
    {"The number can not be divided by 0."},
    {"The global variable $(name) does not exist."},
    {"The statement of global can not be used outside the block of function."},
    {"The operator $(operator) can not be used for strings."},
    {"dummy"},
};