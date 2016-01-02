#include "getToken.h"
#include "analyze.h"
#include "symtab.h"
#include "parse.h"
#include "interCode.h"
#include "tiny.h"

FILE* source; //�����������ļ�
FILE* listing; //����������
FILE * code;
int lineno = 0; // ���ļ��е��к�
int Error;
char tokenString[MAXTOKENLEN+1];
/* allocate and set tracing flags */
int TraceAnalyze = TRUE;
int TraceCode = TRUE;
//������
int main(int argc, char* argv[])
{
	TraceAnalyze = 0;
	TraceCode = 0;
	lineno = 0; 
	if(!(source=fopen("cmm","r"))){
		fprintf(code,"��Ŀ¼���ļ������ڣ�\n");
	}else if(!(code=fopen("pgm","w"))){
		fprintf(code,"�м�����ļ�����ʧ�ܣ�\n");
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
