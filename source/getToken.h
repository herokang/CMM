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

extern FILE* source; //�����������ļ�
extern FILE* listing; //����������
extern int lineno; // ���ļ��е��к�
extern int Error;
extern char tokenString[MAXTOKENLEN+1];

typedef enum 
{
	ENDFILE,ERROR,
	IF,ELSE,WHILE,READ,WRITE,INT,REAL,
	ID,REALNUM,INTEGER,
	//L1,R1,L2,R2,L3,R3�ֱ��Ӧ()[]{}
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
	int arrnum;/*Ĭ��Ϊ0��0��ʾ����ͨ�����������-1��ʾ������Ԫ�����±�Ϊ����
			   ��-2��ʾ������Ԫ�����±�Ϊ������������0���ʾ������Ԫ�ظ���*/
	int arrpos;/*Ĭ��-1������ʱ��-1����󣬱�ʾ����Ԫ��*/
	char arridpos[MAXTOKENLEN];/*Ĭ��Ϊ���ַ��������ǿ�ʱ��ʾ��Ϊ�����±�ı���*/

} TreeNode;

TreeNode * newStmtNode(StmtKind kind);
TreeNode * newExpNode(ExpKind kind);
char * copyString(char * s);
//void printTree( TreeNode * tree );




#endif
