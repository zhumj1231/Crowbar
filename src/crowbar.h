#ifndef PRIVATE_CROWBAR_H_INCLUDED
#define PRIVATE_CROWBAR_H_INCLUDED

// Variable
typedef struct Variable_tag {
	char	*name;
	CRB_Value	value;
	struct 	Variable_tag	*next;
} Variable;

// ParameterList
typedef struct ParameterList_tag {
	char	*name;
	struct 	ParameterList_tag	*next;
} ParameterList;

// StatementList
typedef struct StatementList_tag {
    Statement*	statement;
    struct 	StatementList_tag	*next;
} StatementList;

// Block
typedef struct {
	StatementList 	*statement_list;
} Block;

typedef enum {
	CROWBAR_FUNCTION_DEFINITION = 1,
	NATIVE_FUNCTION_DEFINITION
} FunctionDefinitionType;

typedef struct FunctionDefinition_tag {
	char 	*name;
	FunctionDefinitionType 	type;
	union {
		struct {
			ParameterList 	*parameter;
			Block 	*block;
		} crowbar_f;
		struct {
			CRB_NativeFunctionProc 	*proc;
		} native_f;
	} u;
	struct FunctionDefinition_tag 	*next;
} FunctionDefinition;

struct CRB_Interpreter_tag {
	MEM_Storage 	interpreter_storage;
	MEM_Storage 	execute_storage;
	Variable 	*variable;
	FunctionDefinition 	*function_list;
	StatementList 	*statement_list;
	int 	current_line_number;
};

#endif