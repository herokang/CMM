# CMM语言编译器

本项目为本科时学习《编译原理》时仿照教材实现的简单语言编译器。

相比于教材中的简单tiny语言，增加了单行多行注释、浮点数real类型、bool类型、数组、允许声明时赋值等

##CMM语言的文法： 
    
    program → stmt-sequence
    stmt-sequence → statement{statement}
    statement → declare-stmt | assign-stmt | read-stmt | write-stmt | if-stmt | while-stmt
    declare-stmt → real | int ID [ = exp ] | ID [ integer ] ;
    assign-stmt → ID [ [ ID | integer ] ] [ = exp ]; 
    read-stmt → read ( ID [ [ID | integer ] ] ) ;
    write-stmt → write ( exp ) ;
    if-stmt → if ( exp ) stmt {elseif-stmt} [else-stmt]
    elseif-stmt → else if ( exp ) stmt
    else-stmt → else stmt
    while-stmt → while ( exp ) stmt
    stmt → statement | { stmt-sequence }
    exp → simple-exp[comparison-op simple-exp]
    comparison-op → < | == | <> 
    simple-exp → term{addop term}
    addop → + | - 
    term → factor{mulop factor}
    mulop → * | /
    factor → ( exp ) | realnum | integer | ID [ [ ID | integer ] ] 

其中，保留字 If ; else ; while ; read ; write ; int ; real ;

特殊符号	+  -  *  /  =   <   = =  < >  (  )  ;  {  } [ ] 

注：

1：read , write, if , while等操作后必须匹配 ( ) 。

2：数组下标支持变量或常量，不支持数组表达式，算数表达式，逻辑表达式。

3：EBNF表示法 参照编译原理及实践中一书程序清单 4-8 中Tiny 语言的语法规则构建。

##语义分析检查内容如下：

| | |
| ------------- |:-------------:|
| 变量检错     | 1.变量使用前是否声明         2.变量是否重复声明 |
| 赋值语句检错      | 赋值语句左右是否类型正确 1.非int赋值给int 2.非int也非real 赋值给real 3.左侧非int也非real      |
| If、else、while检错 | if、else if、while中的条件是否是布尔类型      |
| write、read检错 | 1.write后面是否是real或者int类型 2.read后面的变量是否声明过 |

##CMM语言符号表：

| 符号名称        | 含义           | 
| ------------- |:-------------:| 
| VariableName | 变量名称 | 
| Type      | 数据类型 Int 或者 Real 或者 bool 或者 void |  
| ArrayLength | 数组长度 0表示非数组 | 
| Location | 相对内存地址| 
| Line Numbers | 变量被引用的位置 | 

##CMM Machine完全指令集如下表：

###RO指令

格式:  <b>opcode  r,s,t</b>

| 操作码 | 效果
| ------------- |:-------------:| 
| HALT | 停止执行(忽略操作数)
| IN | reg [r]←从标准读入整形值(s和t忽略)
| OUT | reg [r]→标准输出(s和t忽略)
| ADD | reg [r] = reg[s] + reg[t]
| SUB | reg [r] = reg[s] - reg[t]
| MUL | reg [r] = reg[s] * reg[t]
| DIV | reg [r] = reg[s] / reg[t] (可能产生ZERO_DIV)

###RM指令

格式	<b>opcode r,d(s)</b>

(a=d+reg[s];任何对dmem[a]的引用在a<0或a≥DADDR-SIZE时产生DMEM-ERR)

| 操作码	 | 效果
| ------------- |:-------------:| 
| LD     |        	reg[r]=dMem[a] (将a中的值装入r)
| LDA | 	reg[r]=a(将地址a直接装入r)
| LDC    |         	reg[r]=d(将常数d直接装入r,忽略s)
| ST      |        	dMem[a]=reg[r] (将r的值存入位置a)	
| JLT      |       	if(reg[r]<0)reg [PC_REG]=a(如果r小于零转移到a，以下类似)
| JLE       |      	if(reg[r]<=0reg  [PC_REG]=a
| JGE       |      	if(reg[r]>0)reg [PC_REG]=a
| JGT   | 	if(reg[r]>0)reg [PC_REG]=a
| TEQ    | 	if(reg[r]==0)reg [PC_REG]=a
| JNE    | 	if(reg[r]!=0)reg [PC_REG]=a
	


###ARR指令（供数组访问操作）

| 格式 | opcode r,d(s)
| ------------- |:-------------:| 
| WARR |	dMem[reg[arc]+reg[s]] = reg[r]    忽略d,arc为寄存器
| RARR  |  	reg[r] = dMem[reg[arc]+reg[s]]    忽略d,arc为寄存器   


##总结

本项目为CMM语言解释器，使用C++语言实现，基本功能支持if – else 判断，while 循环，支持嵌套使用，允许使用数组，可以进行输入输出。在数据类型上支持整数和浮点数。基本上能够完成基本编程功能。
由于时间的原因，这个项目还存在非常多值得完善的地方，比如：函数功能的实现、指针功能的实现等等。