#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "getToken.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ 
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{ 
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

/* the list of line numbers of the source 
* code in which a variable is referenced
*/
typedef struct LineListRec
{ 
	int lineno;
	struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
* each variable, including name, 
* assigned memory location, and
* the list of line numbers in which
* it appears in the source code
*/
typedef struct BucketListRec
{ 
	char * name;
	ExpType type;
	LineList lines;
	int memloc ; /* memory location for variable */
	int arrnum;//默认为0，数组长度
	struct BucketListRec * next;
} * BucketList;

/* the hash table */
static BucketList hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
* memory locations into the symbol table
* loc = memory location is inserted only the
* first time, otherwise ignored
*/
void st_insert(ExpType t, char * name,int arrnum,int lineno, int loc ){
	int h = hash(name);
	BucketList l =  hashTable[h];
	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL) /* variable not yet in table */
	{ 
		l = (BucketList) malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList) malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;
		l->lines->next = NULL;
		l->next = hashTable[h];
		//
		l->type=t;
		l->arrnum=arrnum;
		hashTable[h] = l; 
	}else /* 理论上不会出现 */
	{ 
		fprintf(listing,"错误！\n");
		exit(0);
	}
}
void st_insert( char * name, int lineno, int loc )
{ 
	int h = hash(name);
	BucketList l =  hashTable[h];
	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL) /* 理论上不会出现 */
	{ 
		fprintf(listing,"错误！\n");
		exit(0);
	}else /* found in table, so just add line number */
	{ 
		LineList t = l->lines;
		while (t->next != NULL) t = t->next;
		t->next = (LineList) malloc(sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next = NULL;
	}
} /* st_insert */

/* Function st_lookup returns the memory 
* location of a variable or -1 if not found
*/
//数组则返回array[0]地址
int st_lookup ( char * name )
{ 
	int h = hash(name);
	BucketList l =  hashTable[h];
	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL) return -1;
	else return l->memloc;
}
int st_lookuparrnum ( char * name )
{ 
	int h = hash(name);
	BucketList l =  hashTable[h];
	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL) return -1;
	else return l->arrnum;
}
ExpType st_lookuptype(char *name){
	int h = hash(name);
	BucketList l =  hashTable[h];
	while ((l != NULL) && (strcmp(name,l->name) != 0))
		l = l->next;
	if (l == NULL){//不应出现
		fprintf(listing,"错误！\n");
		exit(0);
	}else{
		return l->type;
	}
}
/* Procedure printSymTab prints a formatted 
* listing of the symbol table contents 
* to the listing file
*/
void printSymTab(FILE * listing)
{ 
	int i;
	fprintf(listing,"Variable Name  Type  ArrayLength  Location   Line Numbers\n");
	fprintf(listing,"-------------  ----  -----------  --------   ------------\n");
	for (i=0;i<SIZE;++i)
	{ 
		if (hashTable[i] != NULL)
		{ 
			BucketList l = hashTable[i];
			while (l != NULL)
			{ 
				LineList t = l->lines;
				fprintf(listing,"%-14s ",l->name);
				switch(l->type){
				case Void:
					fprintf(listing,"%-4s  ","void");
					break;
				case Integer:
					fprintf(listing,"%-4s  ","int");
					break;
				case Real:
					fprintf(listing,"%-4s  ","real");
					break;
				case Boolean:
					fprintf(listing,"%-4s  ","bool");
					break;
				default:
					fprintf(listing,"%-4s  ","");
					break;
				}
				if(l->arrnum>0){
					fprintf(listing,"%-11d  ",l->arrnum);
				}else{
					fprintf(listing,"%-11s  ","NotArray");
				}
				fprintf(listing,"%-8d  ",l->memloc);
				while (t != NULL)
				{ 
					fprintf(listing,"%4d ",t->lineno);
					t = t->next;
				}
				fprintf(listing,"\n");
				l = l->next;
			}
		}
	}
} /* printSymTab */
