#ifndef __JRPN_H__
#define __JRPN_H__

#include <stdio.h>
#include "JStack.h"
#include "JString.h"
char getOp_Priority(char c1,char c2);
int isOp(char c1);

JStr operates(char* jc1,char op,char* jc2);
JStr evaluateExprs(JStr exprs);
#endif
