#ifndef __JRPN_H__
#define __JRPN_H__

#include <stdio.h>
#include <stdlib.h>
#include "JStack.h"
#include "JString.h"
#include "JList.h"

static char getOp_Priority(char c1,char c2);
static int isOp(char c1);

static JStr operate(const char* jc1,char op,const char* jc2);
static int int_sqrt(int m,int n);

static JStr operateflt(const char* jc1,char op,const char* jc2);
static double flt_sqrt(double m,double n);

static JList* splitExprs(const JStr exprs);
static JStr calcExprs(const JStr exprs,int type);

JStr jrpn_calcFlt(const JStr exprs);
JStr jrpn_calcInt(const JStr exprs);
void jrpn_test();
#endif
