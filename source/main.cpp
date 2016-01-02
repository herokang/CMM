#include "getToken.h"
#include "analyze.h"
#include "symtab.h"
#include "parse.h"
#include "interCode.h"
#include "tiny.h"

FILE* source; //待分析程序文件
FILE* listing; //保存分析结果
FILE * code;
int lineno = 0; // 在文件中的行号
int Error;
char tokenString[MAXTOKENLEN+1];
/* allocate and set tracing flags */
int TraceAnalyze = TRUE;
int TraceCode = TRUE;
//主程序
int main(int argc, char* argv[])
{
	TraceAnalyze = 0;
	TraceCode = 0;
	lineno = 0; 
	if(!(source=fopen("cmm","r"))){
		fprintf(code,"本目录下文件不存在！\n");
	}else if(!(code=fopen("pgm","w"))){
		fprintf(code,"中间代码文件创建失败！\n");
	}else{
		listing=code;
		TreeNode *syntaxTree=parse();
		if (!Error){ 
			buildSymtab(syntaxTree);
			typeCheck(syntaxTree);
		}
		if (!Error){
			codeGen(syntaxTree);
		}
		fclose(code);
		fclose(source);
		fclose(listing);
		if (!Error){
			tiny();
		}

	}
	return 0;
}
