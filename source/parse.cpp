#include "getToken.h"
#include "parse.h"

extern FILE* source; //待分析程序文件
extern FILE* source; //待分析程序文件
extern FILE* listing; //保存分析结果
extern int lineno; // 在文件中的行号
extern int Error;
extern char tokenString[MAXTOKENLEN+1]; 


static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode * stmt_sequence(void);
static TreeNode * statement(void);
static TreeNode * declare_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * if_stmt(void);
static TreeNode * while_stmt(void);

static TreeNode * elseif_stmt(void);
static TreeNode * else_stmt(void);
static TreeNode * stmt(void);

static TreeNode * exp(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * factor(void);

static void syntaxError(char * message)
{ 
	fprintf(listing,"\n>>> ");
	fprintf(listing,"Syntax error at line %d: %s",lineno,message);
	Error = TRUE;
	exit(0);
}

static void match(TokenType expected)
{ 
	if (token == expected) token = getToken();
	else {
		syntaxError("unexpected token -> ");
		printToken(token,tokenString);
		fprintf(listing,"      ");
	    exit(0);
	}
}

TreeNode * stmt_sequence(void)
{ 
	TreeNode * t = statement();
	TreeNode * p = t;
	while ((token!=ENDFILE)&&(token!=R3)&&(token!=ELSE))
	{ 
		TreeNode * q;
		q = statement();
		if (q!=NULL) {
			if (t==NULL) t = p = q;
			else /* now p cannot be NULL either */
			{ 
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}


TreeNode * statement(void)
{ 
	TreeNode * t = NULL;
	switch (token) {
	case REAL:
	case INT:t=declare_stmt();break;
	case ID: t=assign_stmt();break;
	case READ : t = read_stmt(); break;
	case WRITE : t = write_stmt(); break;
	case IF : t = if_stmt(); break;
	case WHILE : t = while_stmt(); break;
	default : syntaxError("unexpected token -> ");
		printToken(token,tokenString);
		token = getToken();
		break;
	} /* end case */
	return t;
}


TreeNode * declare_stmt(void){
	TreeNode * t = newStmtNode(DeclareK);
	if (t!=NULL){
		if(token==REAL){
			t->type=Real;
			match(REAL);
		}else if(token==INT){
			t->type=Integer;
			match(INT);
		}
	}
	char * tmpid=copyString(tokenString);
	match(ID);
	if(token==ASSIGN){
		match(ASSIGN);
		if (t!=NULL){
			t->attr.name = copyString(tmpid);
			t->child[0]=exp();
		}
	}else if(token==L2){
		match(L2);
		char*tmpint=copyString(tokenString);
		match(INTEGER);
		match(R2);
		if (t!=NULL){
			int arrn=atoi(tmpint);
			t->arrnum=arrn;
			t->attr.name = copyString(tmpid);
		}
	}else{
		if (t!=NULL){
			t->attr.name = copyString(tmpid);
		}
	}
	match(SEMI);
	return t;
}

TreeNode * assign_stmt(void)
{ 
	TreeNode * t = newStmtNode(AssignK);
	char * tmpid=copyString(tokenString);
	char tmp[100];//理论上够用于MAXTOKENLEN+2+数字长度
	match(ID);
	if(token==L2){
		match(L2);
		char*tmpint=copyString(tokenString);
		if(token==INTEGER){
			t->arrnum=-1;
			int arrn=atoi(tmpint);
			t->arrpos=arrn;
			match(INTEGER);
		}else if(token==ID){
			t->arrnum=-2;
			strcpy(t->arridpos,tmpint);
			match(ID);
		}
		match(R2);
		tmp[0]='\0';
		strcat(tmp,tmpid);
	}else{
		tmp[0]='\0';
		strcat(tmp,tmpid);
	}
	if (t!=NULL){
		t->attr.name = copyString(tmp);
	}
	match(ASSIGN);
	if (t!=NULL){
		t->child[0]=exp();
	}
	match(SEMI);
	return t;
}

TreeNode * read_stmt(void)
{ 
	TreeNode * t = newStmtNode(ReadK);
	match(READ);
	match(L1);
	char * tmpid;
	tmpid=copyString(tokenString);
	match(ID);
	if(token==L2){
		match(L2);
		char*tmpint=copyString(tokenString);
		if(token==INTEGER){
			t->arrnum=-1;
			int arrn=atoi(tmpint);
			t->arrpos=arrn;
			match(INTEGER);
		}else if(token==ID){
			t->arrnum=-2;
			strcpy(t->arridpos,tmpint);
			match(ID);
		}
		match(R2);
	}
	if (t!=NULL){
		t->attr.name = copyString(tmpid);
	}
	match(R1);
	match(SEMI);
	return t;
}

TreeNode * write_stmt(void)
{ 
	TreeNode * t = newStmtNode(WriteK);
	match(WRITE);
	match(L1);
	if (t!=NULL) t->child[0] = exp();
	match(R1);
	match(SEMI);
	return t;
}

TreeNode * if_stmt(void)
{
	TreeNode * m = newStmtNode(IfK);
	TreeNode * t=m;
	match(IF);
	match(L1);
	if (t!=NULL) t->child[0] = exp();
	match(R1);
	if (t!=NULL) t->child[1] = stmt();
	while(token==ELSE){
		match(ELSE);
		if(token==IF){
			if (t!=NULL){ 
				t->child[2] = elseif_stmt();
				t=t->child[2];
			}
		}else{
			if (t!=NULL) t->child[2] = else_stmt();
			break;
		}
	}
	return m;
}
TreeNode * elseif_stmt(void){
	TreeNode * t = newStmtNode(ElseifK);
	match(IF);
	match(L1);
	if (t!=NULL) t->child[0] = exp();
	match(R1);
	if (t!=NULL) t->child[1] = stmt();
	return t;
}
TreeNode * else_stmt(void){
	TreeNode * t = newStmtNode(ElseK);
	if (t!=NULL) t->child[1] = stmt();
	return t;
}

TreeNode * while_stmt(void){
	TreeNode * t = newStmtNode(WhileK);
	match(WHILE);
	match(L1);
	if (t!=NULL) t->child[0] = exp();
	match(R1);
	if (t!=NULL) t->child[1] = stmt();
	return t;
}

TreeNode * stmt(void){
	TreeNode * t=NULL;
	if(token==L3){
		match(L3);
		t=stmt_sequence();
		match(R3);
	}else{
		t=statement();
	}
	return t;
}

TreeNode * exp(void)
{ 
	TreeNode * t = simple_exp();
	if ((token==LT)||(token==EQ)||(token==NEQ)) {
		TreeNode * p = newExpNode(OpK);
		if (p!=NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t!=NULL)
			t->child[1] = simple_exp();
		t->type=Boolean;
	}
	return t;
}

TreeNode * simple_exp(void)
{ 
	TreeNode * t = term();
	while ((token==PLUS)||(token==MINUS))
	{ 
		TreeNode * p = newExpNode(OpK);
		if (p!=NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();
		}
	}
	return t;
}

TreeNode * term(void)
{ 
	TreeNode * t = factor();
	while ((token==TIMES)||(token==OVER))
	{ 
		TreeNode * p = newExpNode(OpK);
		if (p!=NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

TreeNode * factor(void)
{ 
	TreeNode * t = NULL;
	switch (token) {
	case REALNUM :
		t = newExpNode(ConstK);
		if (t!=NULL){
			t->attr.val = copyString(tokenString);
			t->type=Real;
		}
		match(REALNUM);
		break;
	case INTEGER:
		t = newExpNode(ConstK);
		if (t!=NULL){
			t->attr.val = copyString(tokenString);
			t->type=Integer;
		}
		match(INTEGER);
		break;
	case ID :
		t = newExpNode(IdK);
		char * tmpid;
		tmpid=copyString(tokenString);
		char tmp[100];//理论上够用于MAXTOKENLEN+2+数字长度
		tmp[0]='\0';
		strcat(tmp,tmpid);
		match(ID);
		if(token==L2){
			match(L2);
			char*tmpint=copyString(tokenString);
			if(token==INTEGER){
				int arrn=atoi(tmpint);
				t->arrpos=arrn;
				t->arrnum=-1;
				match(INTEGER);
			}else if(token==ID){
				t->arrnum=-2;
				strcpy(t->arridpos,tmpint);
				match(ID);
			}
			match(R2);
		}
		if (t!=NULL){
			t->attr.name = copyString(tmp);
		}
		break;
	case L1:
		match(L1);
		t = exp();
		match(R1);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token,tokenString);
		token = getToken();
		break;
	}
	return t;
}

TreeNode * parse(void)
{ 
	TreeNode * t;
	token = getToken();
	t = stmt_sequence();
	if (token!=ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}

