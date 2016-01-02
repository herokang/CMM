#include "getToken.h"
#include "analyze.h"
#include "symtab.h"
#include "parse.h"



#ifndef _CODE_H_
#define _CODE_H_

#define  pc 7
#define  mp 6
#define gp 5
#define  ac 0
#define  ac1 1

#define arc 2
#define arc1 3

void emitComment( char * c );
void emitRO( char *op, int r, int s, int t, char *c);
void emitRM( char *op, int r, int d, int s, char *c);
int emitSkip( int howMany);
void emitBackup( int loc);
void emitRestore(void);
void emitRM_Abs( char *op, int r, int a, char * c);

void emitFloat( char *op, int r, double d, int s, char *c);

#endif
