#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "list.h"

#define MAX_STACK 1000

int stack_count;
int scope_count[MAX_STACK];
list *stack[MAX_STACK][MAX_STACK];


int stack_fun_count;
node *stack_fun[MAX_STACK];

/* ****************
    private
*/

node *get_by_name(char *name);

int *get_scope_count() { return &scope_count[stack_count - 1]; }

list **get_last_scope() { return &stack[stack_count - 1][*get_scope_count() - 1]; }

node *find_name_in_list(list *vars, char *name);

/* ************** */

bool has_be_start_before = false;
void start_stack()
{
    stack_count = 0;
    up_stack();

    if (!has_be_start_before)
        stack_fun_count = 0;
    
    has_be_start_before = true;
}

void free_stack()
{
    while (stack_count > 0)
    {
        down_stack();
    }

    stack_count = 0;
    *get_scope_count() = 0;

}

bool up_stack()
{
    if (stack_count >= MAX_STACK)
    {
        printf("error: stack overflow, you reatch the limit of %d\n", MAX_STACK);
        return false;
    }

    stack_count++;
    *get_scope_count() = 0;
    up_scope();

    return true;
}

void down_stack()
{
    if (stack_count < 1)
    {
        printf("internal error: down stack, size < 1\n");
        exit(1);
    }

    while (*get_scope_count() > 0)
    {
        down_scope();
    }
    stack_count--;
}

bool up_scope()
{
    if (*get_scope_count() >= MAX_STACK)
    {
        printf("error: stack overflow, you reatch the limit of %d\n", MAX_STACK);
        return false;
    }

    *get_scope_count() = *get_scope_count() + 1;
    *get_last_scope() = new_list();

    return true;
}

void down_scope()
{
    if (*get_scope_count() < 1)
    {
        printf("internal error: down_scope, size < 1\n");
        exit(1);
    }

    free_list(*get_last_scope());
    *get_scope_count() = *get_scope_count() - 1;
}

void add_stack(node *n)
{
    if (n == NULL || n->type != N_VARIABLE)
    {
        printf("internal error: add_stack\n");
        exit(1);
    }

    if (!add_tail(*get_last_scope(), n))
    {
        printf("internal error: add_stack\n");
        exit(1);
    }
}


void add_fun(node *n) {
    if (n == NULL || n->type != N_FUN)
    {
        printf("internal error: add_fun\n");
        exit(1);
    }

    if (stack_fun_count > MAX_STACK) {
        printf("well this can't actually happend right\n");
        exit(1);
    }

    stack_fun_count++;
    stack_fun[stack_fun_count - 1] = n;

}

char *get_char(char *name)
{
    node *n = get_by_name(name);
    if (n == NULL || n->type != N_VARIABLE || n->data_type != D_CHAR)
    {
        printf("internal error: set_char\n");
        exit(1);
    }
    return n->string;
}

int get_int(char *name)
{
    node *n = get_by_name(name);
    if (n == NULL || n->type != N_VARIABLE || n->data_type != D_INT)
    {
        printf("internal error: get_int\n");
        exit(1);
    }
    return n->number;
}

void set_char(char *name, char *value)
{
    node *n = get_by_name(name);
    if (n == NULL || n->type != N_VARIABLE || n->data_type != D_CHAR)
    {
        printf("internal error: set_char\n");
        exit(1);
    }
    strcpy(n->string, value);
}

void set_int(char *name, int value)
{
    node *n = get_by_name(name);
    if (n == NULL || n->type != N_VARIABLE || n->data_type != D_INT)
    {
        printf("internal error: set_int\n");
        exit(1);
    }
    n->number = value;
}

node *get_fun(char *name, DataType data_type)
{
    node *n;
    int i;
    bool found = false;
    for (i = 0; i < stack_fun_count; i++)
    {
        n = stack_fun[i];
        if (!strcmp(n->name, name)) {
            found = true;
            break;
        }
    }




    if (!found || n->type != N_FUN)
    {
        return NULL;
    }

    if (data_type != D_UNDEFINED && data_type != n->data_type)
    {
        printf("function %s est de type %s. %s attendu\n",
            name, data_type_to_text(n->data_type), 
            data_type_to_text(data_type));
        return NULL;
    }

    return n;
}




DataType check_variable(char *name, DataType data_type)
{

    node *n = get_by_name(name);

    if (n == NULL)
    {
        return D_UNDEFINED;
    }

    if (n->type != N_VARIABLE) {
        printf("'%s' is not a variable\n", name);
        return D_UNDEFINED;
    }

    if (data_type == D_UNDEFINED)
    {
        return n->data_type;
    }

    if (data_type == n->data_type)
    {
        return data_type;
    }
    else
    {
        printf("variable %s est de type %s. %s attendu\n",
            name, data_type_to_text(n->data_type), 
            data_type_to_text(data_type));
        return D_UNDEFINED;
    }
}

node *find_name_in_list(list *vars, char *name)
{

    if (vars == NULL)
    {
        printf("internal error: get_by_name\n");
        exit(1);
    }

    node *tmp = vars->head;

    while (tmp)
    {
        if (!strcmp(tmp->name, name))
            return tmp;
        tmp = tmp->right;
    }

    return NULL;
}


node *get_by_name(char *name)
{
    node *tmp;
    list *l;

    // check globals variables
    if (stack_count > 1)
    {
        l = stack[0][0];
        tmp = find_name_in_list(l, name);
        if (tmp) return tmp;
    }

    for (int scope = 0; scope < *get_scope_count(); scope++)
    {
        l = stack[stack_count - 1][scope];
        tmp = find_name_in_list(l, name);
        if (tmp) return tmp;
    }

    return NULL;
}