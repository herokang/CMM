#ifndef _getToken_H_
#define _getToken_H_

#define MAXTOKENLEN 40

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAXRESERVED 7

extern FILE* source; //待分析程序文件
extern FILE* listing; //保存分析结果
extern int lineno; // 在文件中的行号
extern int Error;
extern char tokenString[MAXTOKENLEN+1];

typedef enum 
{
	ENDFILE,ERROR,
	IF,ELSE,WHILE,READ,WRITE,INT,REAL,
	ID,REALNUM,INTEGER,
	//L1,R1,L2,R2,L3,R3分别对应()[]{}
	ASSIGN,EQ,NEQ,LT,PLUS,MINUS,TIMES,OVER,L1,R1,L2,R2,L3,R3,SEMI
} TokenType;


void printToken( TokenType, const char* );

TokenType getToken(void);


/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum {StmtK,ExpK} NodeKind;
typedef enum {IfK,ElseifK,ElseK,WhileK,DeclareK,AssignK,ReadK,WriteK} StmtKind;
typedef enum {OpK,ConstK,IdK} ExpKind;


/* ExpType is used for type checking */
typedef enum {Void,Integer,Boolean,Real} ExpType;

#define MAXCHILDREN 3

typedef struct treeNode
{ 
	struct treeNode * child[MAXCHILDREN];
	struct treeNode * sibling;
	int lineno;
	NodeKind nodekind;
	union { StmtKind stmt; ExpKind exp;} kind;
	union { TokenType op;
	char * val;
	char * name; } attr;
	ExpType type; /* for type checking of exps */
	int arrnum;/*默认为0，0表示是普通非数组变量，-1表示是数组元素且下标为数字
			   ，-2表示是数组元素且下标为变量，若大于0则表示数组内元素个数*/
	int arrpos;/*默认-1，引用时若-1则错误，表示数组元素*/
	char arridpos[MAXTOKENLEN];/*默认为空字符串，当非空时表示作为数组下标的变量*/

} TreeNode;

TreeNode * newStmtNode(StmtKind kind);
TreeNode * newExpNode(ExpKind kind);
char * copyString(char * s);
//void printTree( TreeNode * tree );




#endif
