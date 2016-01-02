#include "getToken.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;
extern int TraceAnalyze;

static void traverse( TreeNode * t,
	void (* preProc) (TreeNode *),
	void (* postProc) (TreeNode *) )
{
	if (t != NULL)
	{ 
		preProc(t);
		{ 
			int i;
			for (i=0; i < MAXCHILDREN; i++)
				traverse(t->child[i],preProc,postProc);
		}
		postProc(t);
		traverse(t->sibling,preProc,postProc);
	}
}

static void nullProc(TreeNode * t)
{ 
	if (t==NULL) return;
	else return;
}

static void inError(TreeNode * t, char * message)
{ 
	fprintf(listing,"错误行号:%d   错误信息: %s\n",t->lineno,message);
	Error = TRUE;
}

static void insertNode(TreeNode * t)
{ 
	switch (t->nodekind)
	{ 
	case StmtK:
		switch (t->kind.stmt)
		{ 
		case DeclareK:
			if (st_lookup(t->attr.name) == -1){
				/* not yet in table, so treat as new definition */
				st_insert(t->type,t->attr.name,t->arrnum,t->lineno,location++);
				if(t->arrnum>0){
					//数组则多申请元素个数个内存空间
					location=location-1+t->arrnum;
				}
			}else{
				/* 已经在表中，则错误，重复声明*/ 
				inError(t,"变量已存在，重复声明。");
			}
			break;
		case AssignK:
		case ReadK:
			if (st_lookup(t->attr.name) == -1){
				/*错误，变量不存在*/
				inError(t,"变量不存在！");
			}else
				/* already in table, so ignore location, 
				add line number of use only */ 
				st_insert(t->attr.name,t->lineno,0);
			break;
		default:
			break;
		}
		break;
	case ExpK:
		switch (t->kind.exp)
		{ 
		case IdK:
			if (st_lookup(t->attr.name) == -1){
				/*错误，变量不存在*/
				inError(t,"变量不存在！");
			}else
				/* already in table, so ignore location, 
				add line number of use only */ 
				st_insert(t->attr.name,t->lineno,0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}


void buildSymtab(TreeNode * syntaxTree)
{
	traverse(syntaxTree,insertNode,nullProc);
	if (TraceAnalyze)
	{ 
		fprintf(listing,"\n符号表:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode * t, char * message)
{ 
	fprintf(listing,"错误行号：%d   错误信息：类型错误！%s\n",t->lineno,message);
	Error = TRUE;
}


static void checkNode(TreeNode * t)
{ 
	switch (t->nodekind)
	{ 
	case ExpK:
		switch (t->kind.exp)
		{ 
		case OpK:
			if (((t->child[0]->type != Integer)&&(t->child[0]->type !=Real)) ||
				((t->child[1]->type != Integer)&&(t->child[1]->type !=Real)))
				typeError(t,"表达式内类型应为实型或整型。");
			if ((t->attr.op == EQ) ||(t->attr.op == NEQ) || (t->attr.op == LT))
				t->type = Boolean;
			else if((t->child[0]->type ==Real)||(t->child[1]->type ==Real))
				t->type = Real;
			else
				t->type=Integer;
			break;
		case ConstK:
			break;
		case IdK:
			if (st_lookup(t->attr.name) == -1){
				/*错误，变量不存在*/
				inError(t,"变量不存在！");
			}else
				t->type=st_lookuptype(t->attr.name);
			break;
		default:
			break;
		}
		break;
	case StmtK:
		switch (t->kind.stmt)
		{ 
		case IfK:
		case ElseifK:
			if (t->child[0]->type != Boolean)
				typeError(t->child[0],"if判断条件里应为布尔类型。");
			break;
		case AssignK:
			if (st_lookup(t->attr.name) == -1){
				/*错误，变量不存在*/
				inError(t,"变量不存在！");
			}else
				t->type=st_lookuptype(t->attr.name);
			if ((t->child[0]->type != Integer)&&(t->type==Integer))
				typeError(t->child[0],"表达式类型不匹配，右侧结果不可赋值给int类型变量");
			else if((t->child[0]->type != Integer)&&(t->child[0]->type != Real)&&(t->type==Real))
				typeError(t->child[0],"表达式类型不匹配，右侧结果不可赋值给real类型变量");
			else if((t->type!=Real)&&(t->type!=Integer))
				typeError(t->child[0],"暂不支持对左侧类型变量的赋值");
			break;
		case DeclareK:
			if(t->child[0]!=NULL){
				if ((t->child[0]->type != Integer)&&(t->type==Integer))
					typeError(t->child[0],"表达式类型不匹配，右侧结果不可赋值给int类型变量");
				else if((t->child[0]->type != Integer)&&(t->child[0]->type != Real)&&(t->type==Real))
					typeError(t->child[0],"表达式类型不匹配，右侧结果不可赋值给real类型变量");
				else if((t->type!=Real)&&(t->type!=Integer))
					typeError(t->child[0],"暂不支持对左侧类型变量的赋值");
			}
			break;
		case ReadK:
			if (st_lookup(t->attr.name) == -1){
				/*错误，变量不存在*/
				inError(t,"变量不存在！");
			}else
				t->type=st_lookuptype(t->attr.name);
			if ((t->type != Integer)&&(t->type != Real))
				typeError(t->child[0],"read函数参数应为real或int");
			break;
		case WriteK:
			if ((t->child[0]->type != Integer)&&(t->child[0]->type != Real))
				typeError(t->child[0],"write函数参数应为real或int");
			break;
		case WhileK:
			if (t->child[0]->type != Boolean)
				typeError(t->child[1],"while判断条件里应为布尔类型。");
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}
}

void typeCheck(TreeNode * syntaxTree)
{ 
	traverse(syntaxTree,nullProc,checkNode);
}
