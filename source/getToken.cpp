#include "getToken.h"

#define BUFLEN 256

//DFA中状态
//具体参见实验二报告中DFA
typedef enum
{ START,INNUM,INREAL,INID,INIDT,INEQ,INLE,INC,INCO,INCON,INCT,DONE }
StateType;


static char lineBuf[BUFLEN]; // 目前所在行字符串 
static int linepos = 0; // 在LineBuf中的位置 
static int bufsize = 0; // 目前串大小 

//获取下一个字符,行尽则换行,文件结束则终止
static int getNextChar(void)
{
	if (!(linepos < bufsize))
	{	
		lineno++;
		if (fgets(lineBuf,BUFLEN-1,source))
		{
			//fprintf(listing,"%4d: %s",lineno,lineBuf);
			//printf("%4d: %s",lineno,lineBuf);
			bufsize = strlen(lineBuf);
			linepos = 0;
			return lineBuf[linepos++];
		}else
		{	
			return EOF;
		}
	}
	else return lineBuf[linepos++];
}


static struct
{ 
	char* str;
	TokenType tok;
} reservedWords[MAXRESERVED]
= {{"if",IF},{"else",ELSE},{"while",WHILE},
{"read",READ},{"write",WRITE},{"int",INT},
{"real",REAL}};

//查找保留字
static TokenType reservedLookup (char* s)
{ 
	int i;
	for (i=0;i<MAXRESERVED;i++)
		if (!strcmp(s,reservedWords[i].str))
			return reservedWords[i].tok;
	return ID;
}


//本意与课本上基本相同
TokenType getToken(void)
{ 
	char errmes[100];
	errmes[0]='\0';
	int tokenIndex=0;
	TokenType currentToken;
	StateType state = START;
	while (state != DONE)
	{ 
		int c = getNextChar();
		//判断非ASCII字符
		if((c<-1)&&!((state==INCO)||(state==INCON)||(state==INCT))){
			strcpy(errmes,"无法识别的符号");
			state=DONE;
			currentToken = ERROR;
		}
		switch (state)
		{ 
		case START:
		if (isdigit(c)){
			state = INNUM;
			tokenString[tokenIndex++] = (char) c;
		}
		else if (isalpha(c)){
			state = INID;
			tokenString[tokenIndex++] = (char) c;
		}
		else if (c == '=')
			state = INEQ;
		else if (c == '<')
			state = INLE;
		else if (c == '/')
		{ 
			state = INC;
		}
		else
		{ 
			state = DONE;
			switch (c)
			{ 
			case '+': currentToken = PLUS;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '-': currentToken = MINUS;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '*': currentToken = TIMES;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '(': currentToken = L1;
				tokenString[tokenIndex++] = (char) c;
				break;
			case ')': currentToken = R1;
				tokenString[tokenIndex++] = (char) c;
				break;
			case ';': currentToken = SEMI;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '{': currentToken = L3;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '}': currentToken = R3;
				tokenString[tokenIndex++] = (char) c;
				break;
			case '[': currentToken = L2;
				tokenString[tokenIndex++] = (char) c;
				break;
			case ']': currentToken = R2;
				tokenString[tokenIndex++] = (char) c;
				break;
			case ' ':
			case '\t':
			case '\n':
				state=START;
				break;
			case EOF:
				currentToken = ENDFILE;
				break;
			default:
				tokenString[tokenIndex++] = (char) c;
				state=DONE;
				strcpy(errmes,"flag");
				currentToken = ERROR;
				break;
			}
		}
		break;
		case INEQ:
			state = DONE;
			if (c == '='){
				strcpy(tokenString,"==");
				currentToken = EQ;
			}else{ 
				currentToken = ASSIGN;
				strcpy(tokenString,"=");
				linepos--;
			}
			break;
		case INLE:
			state = DONE;
			if (c == '>'){
				strcpy(tokenString,"<>");
				currentToken = NEQ;
			}else{ 
				currentToken = LT;
				strcpy(tokenString,"<");
				linepos--;
			}
			break;
		case INNUM:
			currentToken = INTEGER;
			if('.'==c){
				state=INREAL;
				tokenString[tokenIndex++] = (char) c;
			}else if(isdigit(c)){
				state=INNUM;
				tokenString[tokenIndex++] = (char) c;
			}else{
				state=DONE;
				linepos--;
			}
			break;
		case INID:
			currentToken = ID;
			if('_'==c){
				state=INIDT;
				tokenString[tokenIndex++] = (char) c;
			}else if((isalpha(c))||(isdigit(c))){
				state=INID;
				tokenString[tokenIndex++] = (char) c;
			}else{
				state=DONE;
				linepos--;
			}
			break;
		case INREAL:
			currentToken = REALNUM;
			if(isdigit(c)){
				state=INREAL;
				tokenString[tokenIndex++] = (char) c;
			}else{
				state=DONE;
				linepos--;
			}
			break;
		case INIDT:
			currentToken = ID;
			if('_'==c){
				state=INIDT;
				tokenString[tokenIndex++] = (char) c;
			}else if((isalpha(c))||(isdigit(c))){
				state=INID;
				tokenString[tokenIndex++] = (char) c;
			}else{
				strcpy(errmes,"标识符不可以下划线结尾");
				state=DONE;
				currentToken = ERROR;
			}
			break;
		case INC:
			if (c == EOF)
			{ state = DONE;
			currentToken = ENDFILE;
			}
			else{
				switch(c){
				case '/':
					state=INCT;
					break;
				case '*':
					state=INCO;
					break;
				default:
					currentToken = OVER;
					state=DONE;
					strcpy(tokenString,"/");
					linepos--;
					break;
				}
			}
			break;
		case INCO:
			if (c == EOF)
			{ 
				state = DONE;
				currentToken = ENDFILE;
			}
			else{
				switch(c){
				case '*':
					state=INCON;
					break;
				default:
					state=INCO;
					break;
				}
			}
			break;
		case INCON:
			if (c == EOF)
			{ 
				state = DONE;
				currentToken = ENDFILE;
			}
			else{
				switch(c){
				case '/':
					state=START;
					break;
				default:
					state=INCO;
					break;
				}
			}
			break;
		case INCT:
			if (c == EOF)
			{ state = DONE;
			currentToken = ENDFILE;
			}
			else{
				if(('\n'==c)||('\0'==c))
					state=START;
			}
			break;
		case DONE:
			break;
		default: 
			break;
		}
	}
	if(currentToken == ERROR){
		fprintf(listing,"词法错误！ %s\n",errmes);
		fprintf(listing,"错误位置 行：%d   列：%d\n\n",lineno,linepos);
		return ENDFILE;
	}else{
		tokenString[tokenIndex] = '\0';
		if (currentToken == ID)
			currentToken = reservedLookup(tokenString);
		/*fprintf(listing,"\t%d: ",lineno);
		printf("\t%d: ",lineno);*/
		//printToken(currentToken,tokenString);
	}

	return currentToken;
} 



TreeNode * newStmtNode(StmtKind kind)
{ 
	TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else {
		for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
		t->arrnum=0;
		t->arrpos=-1;
		t->arridpos[0]='\0';
	}
	return t;
}

TreeNode * newExpNode(ExpKind kind)
{ 
	TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
	int i;
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else {
		for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Void;
		t->arrnum=0;
		t->arrpos=-1;
		t->arridpos[0]='\0';
	}
	return t;
}

char * copyString(char * s)
{ 
	int n;
	char * t;
	if (s==NULL) return NULL;
	n = strlen(s)+1;
	t =(char *) malloc(n);
	if (t==NULL)
		fprintf(listing,"Out of memory error at line %d\n",lineno);
	else strcpy(t,s);
	return t;
}

//打印结果
void printToken( TokenType token, const char* tokenString )
{
	switch (token)
	{ 
	case IF:
	case WHILE:
	case ELSE:
	case INT:
	case REAL:
	case READ:
	case WRITE:
		fprintf(listing,"保留字: %s\n",tokenString);
		//printf("保留字: %s\n",tokenString);
		break;
	case ASSIGN: fprintf(listing,"=\n"); break;
	case EQ:     fprintf(listing,"==\n"); break;
	case NEQ:    fprintf(listing,"<>\n"); break;
	case LT:     fprintf(listing,"<\n"); break;
	case PLUS:   fprintf(listing,"+\n"); break;
	case MINUS:  fprintf(listing,"-\n"); break;
	case TIMES:  fprintf(listing,"*\n"); break;
	case OVER:   fprintf(listing,"/\n"); break;
	case L1:     fprintf(listing,"(\n"); break;
	case R1:     fprintf(listing,")\n"); break;
	case L2:     fprintf(listing,"[\n"); break;
	case R2:     fprintf(listing,"]\n"); break;
	case L3:     fprintf(listing,"{\n"); break;
	case R3:     fprintf(listing,"}\n"); break;
	case SEMI:   fprintf(listing,";\n"); break;
		
	case ENDFILE: 
		fprintf(listing,"文件结束\n"); 
		//printf("文件结束\n");
		break;
	case REALNUM:
	case INTEGER:
		fprintf(listing,"数字: %s\n",tokenString);
		//printf("数字: %s\n",tokenString);
		break;
	case ID:
		fprintf(listing,"标识符: %s\n",tokenString);
		//printf("标识符: %s\n",tokenString);
		break;
	default:
		break;
	}
}