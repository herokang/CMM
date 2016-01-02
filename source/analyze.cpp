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
	fprintf(listing,"�����к�:%d   ������Ϣ: %s\n",t->lineno,message);
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
					//�����������Ԫ�ظ������ڴ�ռ�
					location=location-1+t->arrnum;
				}
			}else{
				/* �Ѿ��ڱ��У�������ظ�����*/ 
				inError(t,"�����Ѵ��ڣ��ظ�������");
			}
			break;
		case AssignK:
		case ReadK:
			if (st_lookup(t->attr.name) == -1){
				/*���󣬱���������*/
				inError(t,"���������ڣ�");
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
				/*���󣬱���������*/
				inError(t,"���������ڣ�");
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
		fprintf(listing,"\n���ű�:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode * t, char * message)
{ 
	fprintf(listing,"�����кţ�%d   ������Ϣ�����ʹ���%s\n",t->lineno,message);
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
				typeError(t,"���ʽ������ӦΪʵ�ͻ����͡�");
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
				/*���󣬱���������*/
				inError(t,"���������ڣ�");
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
				typeError(t->child[0],"if�ж�������ӦΪ�������͡�");
			break;
		case AssignK:
			if (st_lookup(t->attr.name) == -1){
				/*���󣬱���������*/
				inError(t,"���������ڣ�");
			}else
				t->type=st_lookuptype(t->attr.name);
			if ((t->child[0]->type != Integer)&&(t->type==Integer))
				typeError(t->child[0],"���ʽ���Ͳ�ƥ�䣬�Ҳ������ɸ�ֵ��int���ͱ���");
			else if((t->child[0]->type != Integer)&&(t->child[0]->type != Real)&&(t->type==Real))
				typeError(t->child[0],"���ʽ���Ͳ�ƥ�䣬�Ҳ������ɸ�ֵ��real���ͱ���");
			else if((t->type!=Real)&&(t->type!=Integer))
				typeError(t->child[0],"�ݲ�֧�ֶ�������ͱ����ĸ�ֵ");
			break;
		case DeclareK:
			if(t->child[0]!=NULL){
				if ((t->child[0]->type != Integer)&&(t->type==Integer))
					typeError(t->child[0],"���ʽ���Ͳ�ƥ�䣬�Ҳ������ɸ�ֵ��int���ͱ���");
				else if((t->child[0]->type != Integer)&&(t->child[0]->type != Real)&&(t->type==Real))
					typeError(t->child[0],"���ʽ���Ͳ�ƥ�䣬�Ҳ������ɸ�ֵ��real���ͱ���");
				else if((t->type!=Real)&&(t->type!=Integer))
					typeError(t->child[0],"�ݲ�֧�ֶ�������ͱ����ĸ�ֵ");
			}
			break;
		case ReadK:
			if (st_lookup(t->attr.name) == -1){
				/*���󣬱���������*/
				inError(t,"���������ڣ�");
			}else
				t->type=st_lookuptype(t->attr.name);
			if ((t->type != Integer)&&(t->type != Real))
				typeError(t->child[0],"read��������ӦΪreal��int");
			break;
		case WriteK:
			if ((t->child[0]->type != Integer)&&(t->child[0]->type != Real))
				typeError(t->child[0],"write��������ӦΪreal��int");
			break;
		case WhileK:
			if (t->child[0]->type != Boolean)
				typeError(t->child[1],"while�ж�������ӦΪ�������͡�");
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
