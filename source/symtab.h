#include"getToken.h"
#ifndef _SYMTAB_H_
#define _SYMTAB_H_

void st_insert( ExpType t,char * name, int arrnum,int lineno, int loc );
void st_insert( char * name, int lineno, int loc );

int st_lookup ( char * name );

ExpType st_lookuptype(char *name);
int st_lookuparrnum ( char * name );

void printSymTab(FILE * listing);

#endif
