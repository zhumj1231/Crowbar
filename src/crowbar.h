#ifndef PRIVATE_CROWBAR_H_INCLUDED
#define PRIVATE_CROWBAR_H_INCLUDED

typedef struct Variable_tag {
	char *name;
	CRB_Value value;
	struct Variable_tag *next;
} Variable;

struct CRB_Interpreter_tag {
	MEM_Storage interpreter_storage;
	MEM_Storage execute_storage;
	Variable *variable;
	FunctionDefinition *function_list;
	StatementList *statement_list;
	int current_line_number;
};

#endif