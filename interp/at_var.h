#ifndef AT_VAR_H
#define AT_VAR_H

#include "at_interp.h"

void add_var(char name[3], struct value v, char *err);
struct value get_var(char name[3], char *err);

#endif

