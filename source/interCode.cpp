#include "interCode.h"

extern int TraceCode;
extern FILE * code;

/* tmpOffset is the memory offset for temps
It is decremented each time a temp is
stored, and incremeted when loaded again
*/
static int tmpOffset = 0;

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{ 
	TreeNode * p1, * p2, * p3;
	int savedLoc1,savedLoc2,currentLoc;
	int loc;
	switch (tree->kind.stmt) {
	case ElseifK:
	case IfK :
		if (TraceCode) emitComment("-> if") ;
		p1 = tree->child[0] ;
		p2 = tree->child[1] ;
		p3 = tree->child[2] ;
		/* generate code for test expression */
		//判断条件
		cGen(p1);
		savedLoc1 = emitSkip(1) ; //false跳转到else、elseif或者结束

		emitComment("if: jump to else belongs here");

		/* recurse on then part */
		//成功执行部分
		cGen(p2);

		
		if(p2==NULL){
			//无else
			currentLoc = emitSkip(0) ;
			emitBackup(savedLoc1);
			emitRM_Abs("JEQ",ac,currentLoc,"if失败跳转到结束");
			emitRestore();
		}else{
			//有else 或者elseif
			savedLoc2 = emitSkip(1) ; //if成功,执行语句后跳转到结束

			//可以计算if失败跳转位置了
			currentLoc = emitSkip(0) ;
			emitBackup(savedLoc1);
			emitRM_Abs("JEQ",ac,currentLoc,"if失败跳转到结束");
			emitRestore();

			cGen(p3);
			//计算if成功执行语句后跳转位置
			currentLoc = emitSkip(0) ;
			emitBackup(savedLoc2) ;
			emitRM_Abs("LDA",pc,currentLoc,"jmp to end") ;
			emitRestore();

		}
		if (TraceCode)  emitComment("<- if") ;
		break; /* if_k */

	case ElseK:
		if (TraceCode) emitComment("-> else") ;
		p2 = tree->child[1] ;
		cGen(p2);
		if (TraceCode) emitComment("-> else") ;
		break;
		
	case WhileK:
		if (TraceCode) emitComment("-> while") ;
		//p1为判断条件 p2为循环体
		p1 = tree->child[0] ;
		p2 = tree->child[1] ;
		//保存此位置供跳转用
		savedLoc2 = emitSkip(0);
		//判断条件
		cGen(p1);
		savedLoc1 = emitSkip(1); //计算了循环体后再决定跳转到哪一步，故先保存目前位置并跳一位
		//循环体
		cGen(p2);
		//无条件跳转到判断条件前
		currentLoc = emitSkip(0) ;
		emitRM("LDA",pc,savedLoc2-currentLoc-1,pc,"到此无条件跳转到while判断条件前");
		//打印判断失败后的跳转位置
		currentLoc = emitSkip(0) ;
		emitBackup(savedLoc1);
		emitRM_Abs("JEQ",ac,currentLoc,"while条件失败则跳转到此处");
		emitRestore();
		if (TraceCode)  emitComment("<- while") ;
		break; /* while */

	case DeclareK:
		if(tree->child[0]!=NULL){
			if (TraceCode) emitComment("-> declare and assign") ;
			/* generate code for rhs */
			cGen(tree->child[0]);
			/* now store value */
			loc = st_lookup(tree->attr.name);

			//处理数组存储
			if(tree->arrnum==-1){
				if(tree->arrpos<0){
					fprintf(code,"BUG:数组越界或下标数字非法");
					exit(0);
				}else{
					loc=loc+tree->arrpos;  //直接定位到数组元素存储空间
					emitRM("ST",ac,loc,gp,"保存到数组元素");
				}
			}else if(tree->arrnum==-2){
				//提取下标变量值到arc
				int xbloc=st_lookup(tree->arridpos);
				emitRM("LD",arc,xbloc,gp,"加载下标变量值");
				emitRM("LDC",arc1,loc,arc1,"加载数组开始地址");
				emitRO("ADD",arc,arc1,arc,"计算实际地址偏移量");
				emitRM("WARR",ac,0,gp,"数组元素赋值");

			}else if(tree->arrnum==0){
				emitRM("ST",ac,loc,gp,"保存到变量");
			}else{
				fprintf(code,"BUG:声明有问题");
				exit(0);
			}
			//emitRM("ST",ac,loc,gp,"declare and assign: store value");
			if (TraceCode)  emitComment("<- declare and assign") ;
		}
		break;

	case AssignK:
		if (TraceCode) emitComment("-> assign") ;
		/* generate code for rhs */
		cGen(tree->child[0]);
		/* now store value */
		loc = st_lookup(tree->attr.name);

		//处理数组存储
		if(tree->arrnum==-1){
			if(tree->arrpos<0||(tree->arrpos >= st_lookuparrnum(tree->attr.name))){
				fprintf(code,"BUG:数组越界或下标数字非法");
				exit(0);
			}else{
				loc=loc+tree->arrpos;  //直接定位到数组元素存储空间
				emitRM("ST",ac,loc,gp,"read: store value");
			}
		}else if(tree->arrnum==-2){
			//提取下标变量值到arc
			int xbloc=st_lookup(tree->arridpos);
			emitRM("LD",arc,xbloc,gp,"加载下标变量值");
			emitRM("LDC",arc1,loc,arc1,"加载数组开始地址");
			emitRO("ADD",arc,arc1,arc,"计算实际地址偏移量");
			emitRM("WARR",ac,0,gp,"数组元素赋值");

		}else if(tree->arrnum==0){
			emitRM("ST",ac,loc,gp,"read: store value");
		}else{
			fprintf(code,"BUG:赋值语句有问题");
			exit(0);
		}
		//emitRM("ST",ac,loc,gp,"assign: store value");
		if (TraceCode)  emitComment("<- assign") ;
		break; /* assign_k */

	case ReadK:
		emitRO("IN",ac,0,0,"read integer value");
		loc = st_lookup(tree->attr.name);

		//处理数组存储
		if(tree->arrnum==-1){
			if(tree->arrpos<0||(tree->arrpos >= st_lookuparrnum(tree->attr.name))){
				fprintf(code,"BUG:数组越界或下标数字非法");
				exit(0);
			}else{
				loc=loc+tree->arrpos;  //直接定位到数组元素存储空间
				emitRM("ST",ac,loc,gp,"read: store value");
			}
		}else if(tree->arrnum==-2){
			//提取下标变量值到arc
			int xbloc=st_lookup(tree->arridpos);
			emitRM("LD",arc,xbloc,gp,"加载下标变量值");
			emitRM("LDC",arc1,loc,arc1,"加载数组开始地址");
			emitRO("ADD",arc,arc1,arc,"计算实际地址偏移量");
			emitRM("WARR",ac,0,gp,"读入数组");

		}else if(tree->arrnum==0){
			emitRM("ST",ac,loc,gp,"read: store value");
		}else{
			fprintf(code,"BUG:读写函数参数有问题");
			exit(0);
		}

		break;
	case WriteK:
		/* generate code for expression to write */
		cGen(tree->child[0]);
		/* now output it */
		emitRO("OUT",ac,0,0,"write ac");
		break;
	default:
		break;
	}
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{ 
	int loc;
	TreeNode * p1, * p2;
	switch (tree->kind.exp) {
	case ConstK :
		if (TraceCode) emitComment("-> Const") ;
		if(tree->type==Integer){
			emitRM("LDC",ac,atoi(tree->attr.val),0,"load const int");
		}else if(tree->type==Real){
			emitFloat("LDC",ac,atof(tree->attr.val),0,"load const real");
		}
		if (TraceCode)  emitComment("<- Const") ;
		break; /* ConstK */

	case IdK :
		if (TraceCode) emitComment("-> Id") ;
		loc = st_lookup(tree->attr.name);

		//处理数组读取
		if(tree->arrnum==-1){
			if(tree->arrpos<0||(tree->arrpos >= st_lookuparrnum(tree->attr.name))){
				fprintf(code,"BUG:数组越界或下标数字非法");
				exit(0);
			}else{
				loc=loc+tree->arrpos;  //直接定位到数组元素存储空间
				emitRM("LD",ac,loc,gp,"加载常量下标数组元素");
			}
		}else if(tree->arrnum==-2){
			//提取下标变量值到arc
			int xbloc=st_lookup(tree->arridpos);
			emitRM("LD",arc,xbloc,gp,"加载下标变量值");
			emitRM("LDC",arc1,loc,arc1,"加载数组开始地址");
			emitRO("ADD",arc,arc1,arc,"计算实际地址偏移量");
			emitRM("RARR",ac,0,gp,"取出数组元素");
		}else if(tree->arrnum==0){
			emitRM("LD",ac,loc,gp,"普通变量直接读取");
		}else{
			fprintf(code,"BUG:表达式内变量有问题");
			exit(0);
		}

		if (TraceCode)  emitComment("<- Id") ;
		break; /* IdK */

	case OpK :
		if (TraceCode) emitComment("-> Op") ;
		p1 = tree->child[0];
		p2 = tree->child[1];
		/* gen code for ac = left arg */
		cGen(p1);
		/* gen code to push left operand */
		emitRM("ST",ac,tmpOffset--,mp,"op: push left");
		/* gen code for ac = right operand */
		cGen(p2);
		/* now load left operand */
		emitRM("LD",ac1,++tmpOffset,mp,"op: load left");
		switch (tree->attr.op) {
		case PLUS :
			emitRO("ADD",ac,ac1,ac,"op +");
			break;
		case MINUS :
			emitRO("SUB",ac,ac1,ac,"op -");
			break;
		case TIMES :
			emitRO("MUL",ac,ac1,ac,"op *");
			break;
		case OVER :
			emitRO("DIV",ac,ac1,ac,"op /");
			break;
		case LT :
			emitRO("SUB",ac,ac1,ac,"op <") ;
			emitRM("JLT",ac,2,pc,"br if true") ;
			emitRM("LDC",ac,0,ac,"false case") ;
			emitRM("LDA",pc,1,pc,"unconditional jmp") ;
			emitRM("LDC",ac,1,ac,"true case") ;
			break;
		case EQ :
			//ac==ac1 返回true
			emitRO("SUB",ac,ac1,ac,"op ==") ;
			emitRM("JEQ",ac,2,pc,"br if true");
			emitRM("LDC",ac,0,ac,"false case") ;
			emitRM("LDA",pc,1,pc,"unconditional jmp") ;
			emitRM("LDC",ac,1,ac,"true case") ;
			break;
		case NEQ:
			//ac!=ac1 返回true
			emitRO("SUB",ac,ac1,ac,"op !=") ;
			emitRM("JNQ",ac,2,pc,"br if true");
			emitRM("LDC",ac,0,ac,"false case") ;
			emitRM("LDA",pc,1,pc,"unconditional jmp") ;
			emitRM("LDC",ac,1,ac,"true case") ;
		default:
			emitComment("BUG: Unknown operator");
			break;
		} /* case op */
		if (TraceCode)  emitComment("<- Op") ;
		break; /* OpK */

	default:
		break;
	}
} /* genExp */

/* Procedure cGen recursively generates code by
* tree traversal
*/
static void cGen( TreeNode * tree)
{ 
	if (tree != NULL){ 
		switch (tree->nodekind) {
		case StmtK:
			genStmt(tree);
			break;
		case ExpK:
			genExp(tree);
			break;
		default:
			break;
		}
		cGen(tree->sibling);
	}
}


void codeGen(TreeNode * syntaxTree)
{ 
	emitComment("TINY Compilation to TM Code");
	/* generate standard prelude */
	emitComment("Standard prelude:");
	emitRM("LD",mp,0,ac,"load maxaddress from location 0");
	emitRM("ST",ac,0,ac,"clear location 0");
	emitComment("End of standard prelude.");
	/* generate code for TINY program */
	cGen(syntaxTree);
	/* finish */
	emitComment("End of execution.");
	emitRO("HALT",0,0,0,"");
}
