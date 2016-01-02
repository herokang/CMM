#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tiny.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/******* const *******/
#define   IADDR_SIZE  1024 /* increase for large programs */
#define   DADDR_SIZE  1024 /* increase for large programs */
#define   NO_REGS 8
#define   PC_REG  7

#define   LINESIZE  121
#define   WORDSIZE  20

//定义arc
#define arc 2
FILE* fileIn;
FILE* fileOut;
FILE *pgm  ;

/******* type  *******/

typedef enum {
	opclRR,     /* reg operands r,s,t */
	opclRM,     /* reg r, mem d+s */
	opclRA,      /* reg r, int d+s */
	opclARR,    //下标为变量的数组 
	//opclFL    //浮点数操作
} OPCLASS;

typedef enum {
	/* RR instructions */
	opHALT,    /* RR     halt, operands are ignored */
	opIN,      /* RR     read into reg(r); s and t are ignored */
	opOUT,     /* RR     write from reg(r), s and t are ignored */
	opADD,    /* RR     reg(r) = reg(s)+reg(t) */
	opSUB,    /* RR     reg(r) = reg(s)-reg(t) */
	opMUL,    /* RR     reg(r) = reg(s)*reg(t) */
	opDIV,    /* RR     reg(r) = reg(s)/reg(t) */
	opRRLim,   /* limit of RR opcodes */

	/* RM instructions */
	opLD,      /* RM     reg(r) = mem(d+reg(s)) */
	opST,      /* RM     mem(d+reg(s)) = reg(r) */
	opRMLim,   /* Limit of RM opcodes */

	/* RA instructions */
	opLDA,     /* RA     reg(r) = d+reg(s) */
	opLDC,     /* RA     reg(r) = d ; reg(s) is ignored */
	opJLT,     /* RA     if reg(r)<0 then reg(7) = d+reg(s) */
	opJLE,     /* RA     if reg(r)<=0 then reg(7) = d+reg(s) */
	opJGT,     /* RA     if reg(r)>0 then reg(7) = d+reg(s) */
	opJGE,     /* RA     if reg(r)>=0 then reg(7) = d+reg(s) */
	opJEQ,     /* RA     if reg(r)==0 then reg(7) = d+reg(s) */
	opJNE,     /* RA     if reg(r)!=0 then reg(7) = d+reg(s) */
	opRALim,    /* Limit of RA opcodes */

	//opclARR
	rARR,
	wARR,
	opARRLim,

	//opclFL
	//ldFL,
	//opFLLim
} OPCODE;

typedef enum {
	srOKAY,
	srHALT,
	srIMEM_ERR,
	srDMEM_ERR,
	srZERODIVIDE
} STEPRESULT;

typedef struct {
	int iop  ;
	int iarg1  ;
	double iarg2  ;
	int iarg3  ;
} INSTRUCTION;

/******** vars ********/
int traceflag = TRUE;
int icountflag = TRUE;

INSTRUCTION iMem [IADDR_SIZE];
double dMem [DADDR_SIZE];
double reg [NO_REGS];

char * opCodeTab[]
= {"HALT","IN","OUT","ADD","SUB","MUL","DIV","????",
	/* RR opcodes */
	"LD","ST","????", /* RM opcodes */
	"LDA","LDC","JLT","JLE","JGT","JGE","JEQ","JNE","????",
	/* RA opcodes */
	"RARR","WARR","????"/*,
	"LDRR","????"*/
};

char * stepResultTab[]
= {"OK","Halted","Instruction Memory Fault",
	"Data Memory Fault","Division by 0"
};


char in_Line[LINESIZE] ;
int lineLen ;
int inCol  ;
double num  ;
char word[WORDSIZE] ;
char ch  ;
int done  ;

/********************************************/
int opClass( int c )
{ 
	if      ( c <= opRRLim) return ( opclRR );
	else if ( c <= opRMLim) return ( opclRM );
	else if ( c <= opRALim) return ( opclRA );
	else if ( c <= opARRLim) return( opclARR);
	//else return( opclFL);
} /* opClass */


/********************************************/
void getCh (void)
{ 
	if (++inCol < lineLen)
		ch = in_Line[inCol] ;
	else ch = ' ' ;
} /* getCh */
/********************************************/
int nonBlank (void)
{ 

	while ((inCol < lineLen)
		&& (in_Line[inCol] == ' ') )
		inCol++ ;
	if (inCol < lineLen)
	{ 
		ch = in_Line[inCol] ;
		return TRUE ; }
	else
	{ 
		ch = ' ' ;
		return FALSE ; }
} /* nonBlank */
int getNum (void)
{ 
	int sign;
	double term;
	int temp = FALSE;
	num = 0 ;
	do
	{ 
		sign = 1;
		while ( nonBlank() && ((ch == '+') || (ch == '-')) )
		{ 
			temp = FALSE ;
			if (ch == '-')  sign = - sign ;
			getCh();
		}
		term = 0 ;
		nonBlank();
		while (isdigit(ch))
		{ 
			temp = TRUE ;
			term = term * 10 + ( ch - '0' ) ;
			getCh();
		}
		num = num + (term * sign) ;

		//读小数部分
		if(ch=='.'){
			term=0;
			int ff=1;
			getCh();
			while (isdigit(ch))
			{ 
				ff=ff*10;
				temp = TRUE ;
				term = term * 10 + ( ch - '0' ) ;
				getCh();
			}
			num = num + (term/ff )* sign ;
		}
	} while ( (nonBlank()) && ((ch == '+') || (ch == '-')) ) ;
	return temp;
} /* getNum */



/********************************************/
int getWord (void)
{ 
	int temp = FALSE;
	int length = 0;
	if (nonBlank ())
	{ 
		while (isalnum(ch))
		{ 
			if (length < WORDSIZE-1) word [length++] =  ch ;
			getCh() ;
		}
		word[length] = '\0';
		temp = (length != 0);
	}
	return temp;
} /* getWord */

/********************************************/
int skipCh ( char c  )
{ 
	int temp = FALSE;
	if ( nonBlank() && (ch == c) )
	{ 
		getCh();
		temp = TRUE;
	}
	return temp;
} /* skipCh */

/********************************************/
int error( char * msg, int lineNo, int instNo)
{ 
	fprintf(fileOut,"Line %d",lineNo);
	if (instNo >= 0) fprintf(fileOut," (Instruction %d)",instNo);
	fprintf(fileOut,"   %s\n",msg);
	return FALSE;
} /* error */

/********************************************/
int readInstructions (void)
{ 
	int op;
	int arg1, arg3;
	double arg2;
	int loc, regNo, lineNo;
	for (regNo = 0 ; regNo < NO_REGS ; regNo++)
		reg[regNo] = 0 ;
	dMem[0] = DADDR_SIZE - 1 ;
	for (loc = 1 ; loc < DADDR_SIZE ; loc++)
		dMem[loc] = 0 ;
	for (loc = 0 ; loc < IADDR_SIZE ; loc++)
	{ 
		iMem[loc].iop = opHALT ;
		iMem[loc].iarg1 = 0 ;
		iMem[loc].iarg2 = 0 ;
		iMem[loc].iarg3 = 0 ;
	}
	lineNo = 0 ;
	while (! feof(pgm))
	{
		fgets( in_Line, LINESIZE-2, pgm  ) ;
		inCol = 0 ; 
		lineNo++;
		lineLen = strlen(in_Line)-1 ;
		if (in_Line[lineLen]=='\n')
			in_Line[lineLen] = '\0' ;
		else 
			in_Line[++lineLen] = '\0';
		if ( (nonBlank()) && (in_Line[inCol] != '*') )
		{ 
			if (! getNum())
				return error("Bad location", lineNo,-1);
			loc = num;
			if (loc > IADDR_SIZE)
				return error("Location too large",lineNo,loc);
			if (! skipCh(':'))
				return error("Missing colon", lineNo,loc);
			if (! getWord ())
				return error("Missing opcode", lineNo,loc);
			op = opHALT ;
			while ((op < opARRLim)&&(strncmp(opCodeTab[op], word, 4) != 0) )
				op++;
			if (strncmp(opCodeTab[op], word, 4) != 0)
				return error("Illegal opcode", lineNo,loc);
			switch ( opClass(op) )
			{ 
			case opclRR :
				/***********************************/
				if ( (! getNum ()) || (num < 0) || (num >= NO_REGS) )
					return error("Bad first register", lineNo,loc);
				arg1 = num;
				if ( ! skipCh(','))
					return error("Missing comma", lineNo, loc);
				if ( (! getNum ()) || (num < 0) || (num >= NO_REGS) )
					return error("Bad second register", lineNo, loc);
				arg2 = num;
				if ( ! skipCh(',')) 
					return error("Missing comma", lineNo,loc);
				if ( (! getNum ()) || (num < 0) || (num >= NO_REGS) )
					return error("Bad third register", lineNo,loc);
				arg3 = num;
				break;

			case opclRM :
			case opclRA :
			case opclARR:
			//case opclFL:
				/***********************************/
				if ( (! getNum ()) || (num < 0) || (num >= NO_REGS) )
					return error("Bad first register", lineNo,loc);
				arg1 = num;
				if ( ! skipCh(','))
					return error("Missing comma", lineNo,loc);
				if (! getNum ())
					return error("Bad displacement", lineNo,loc);
				arg2 = num;
				if ( ! skipCh('(') && ! skipCh(',') )
					return error("Missing LParen", lineNo,loc);
				if ( (! getNum ()) || (num < 0) || (num >= NO_REGS))
					return error("Bad second register", lineNo,loc);
				arg3 = num;
				break;
			}
			iMem[loc].iop = op;
			iMem[loc].iarg1 = arg1;
			iMem[loc].iarg2 = arg2;
			iMem[loc].iarg3 = arg3;
		}
	}
	return TRUE;
} /* readInstructions */


/********************************************/
STEPRESULT stepTM (void)
{ 
	INSTRUCTION currentinstruction  ;
	int pc  ;
	int r,s,t,m  ;
	int ok ;

	pc = reg[PC_REG] ;
	if ( (pc < 0) || (pc > IADDR_SIZE)  )
		return srIMEM_ERR ;
	reg[PC_REG] = pc + 1 ;
	currentinstruction = iMem[ pc ] ;
	switch (opClass(currentinstruction.iop) )
	{ 
	case opclRR :
		/***********************************/
		r = currentinstruction.iarg1 ;
		s = currentinstruction.iarg2 ;
		t = currentinstruction.iarg3 ;
		break;

	case opclRM :
		/***********************************/
		r = currentinstruction.iarg1 ;
		s = currentinstruction.iarg3 ;
		m = currentinstruction.iarg2 + reg[(int)s] ;
		if ( (m < 0) || (m > DADDR_SIZE))
			return srDMEM_ERR ;
		break;

	case opclRA :
		/***********************************/
		r = currentinstruction.iarg1 ;
		s = currentinstruction.iarg3 ;
		m = currentinstruction.iarg2 + reg[(int)s] ;
		break;

	case opclARR:
		r = currentinstruction.iarg1 ;
		m = reg[arc] + reg[(int)currentinstruction.iarg3] ;
		break;
	} /* case */

	switch ( currentinstruction.iop)
	{ /* RR instructions */
	case opHALT :
		/***********************************/
		//printf("HALT: %1d,%1d,%1d\n",r,s,t);
		return srHALT ;
		/* break; */

	case opIN :
		/***********************************/
		ok = fscanf(fileIn,"%lf",&num);
		if ( ! ok ){
			fprintf(fileOut,"输入不合法！");
			exit(0);
		}else 
			reg[r] = num;
		break;

	case opOUT :  
		fprintf (fileOut,"%f\n", reg[r] ) ;
		//printf ("%f\n", reg[r] ) ;
		break;
	case opADD :  reg[r] = reg[s] + reg[t] ;  break;
	case opSUB :  reg[r] = reg[s] - reg[t] ;  break;
	case opMUL :  reg[r] = reg[s] * reg[t] ;  break;

	case opDIV :
		/***********************************/
		if ( reg[t] != 0 ) reg[r] = reg[s] / reg[t];
		else return srZERODIVIDE ;
		break;

		/*************** RM instructions ********************/
	case opLD :    reg[r] = dMem[m] ;  break;
	case opST :    dMem[m] = reg[r] ;  break;

		/*************** RA instructions ********************/
	case opLDA :    reg[r] = m ; break;
	case opLDC :    reg[r] = currentinstruction.iarg2 ;   break;
	case opJLT :    if ( reg[r] <  0 ) reg[PC_REG] = m ; break;
	case opJLE :    if ( reg[r] <=  0 ) reg[PC_REG] = m ; break;
	case opJGT :    if ( reg[r] >  0 ) reg[PC_REG] = m ; break;
	case opJGE :    if ( reg[r] >=  0 ) reg[PC_REG] = m ; break;
	case opJEQ :    if ( reg[r] == 0 ) reg[PC_REG] = m ; break;
	case opJNE :    if ( reg[r] != 0 ) reg[PC_REG] = m ; break;

		//ARR
	case rARR:   reg[r] = dMem[m] ;  break;
	case wARR:   dMem[m] = reg[r] ;  break;

		/* end of legal instructions */
	} /* case */
	return srOKAY ;
} /* stepTM */

/********************************************/
int doCommand (void)
{ 
	int stepcnt=0;
	int stepResult = srOKAY;
	while (stepResult == srOKAY){ 
		stepResult = stepTM ();
		stepcnt++;
	}
	//if ( icountflag )
	//	printf("Number of instructions executed = %d\n",stepcnt);
	//printf( "%s\n",stepResultTab[stepResult] );
	return TRUE;
} /* doCommand */


/********************************************/
/* E X E C U T I O N   B E G I N S   H E R E */
/********************************************/

int tiny()
{
	fileIn=fopen("fi","r");
	fileOut=fopen("fo","w");
	pgm = fopen("pgm","r");
	if (pgm == NULL)
	{ 
		fprintf(fileOut,"file pgm not found\n");
		exit(1);
	}
	if (fileIn == NULL)
	{ 
		fprintf(fileOut,"file in not found\n");
		exit(1);
	}

	if (!readInstructions ())
		exit(1) ;
	doCommand ();
	//printf("Simulation done.\n");
	fclose(fileIn);
	fclose(fileOut);
	fclose(pgm);
	return 0;
}
