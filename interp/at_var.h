#ifndef AT_VAR_H
#define AT_VAR_H

#include "at_interp.h"

void add_var(char name[3], struct value v, char const **err);
struct value get_var(char name[3], char const **err);

#endif

