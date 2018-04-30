#ifndef _definitions_h
#define _definitions_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// files needed
#define PRE_ASM "pre_assembly.txt"
#define AMEND "jump_amendments.txt"
#define ASM_FINAL "assembly_final.txt"
#define VM_OUT "vm_out.txt"
#define VM_IN  "vm_in.txt"

// needed in vm.c
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3
#define MAX_INSTRUCTION_LEN 100

// needed in lex.c
#define MAX_IDENT_LEN 11
#define MAX_NUM_LEN 5
#define NUM_SPECIAL_SYM 13
#define NUM_RES_WORDS 15

// needed in parser.c
#define MAX_SYMBOL_TABLE_SIZE 101 // make prime to avoid infinite loop
#define CONST 1
#define VAR 2
#define PROC 3



// needed in vm.c
typedef struct instruction {

	int op;      // opcode
	int r;       // register #
	int L;       // L
	int M;       // M

} instruction;

typedef struct registers {

	instruction ir;	

	int bp;
	int sp;
	int pc;
	int rf[16];
	int haltFlag;

	int inputCount;

} registers;

typedef struct actRecords {
	
	int size;
	int bases[MAX_STACK_HEIGHT/4];

} actRecords;


// needed in lex.c
char specialSymbols[13] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};

//char *reservedWords[] = {"null", "begin", "call", "const", "do", "else", "end", "if", 
//			 "odd", "procedure", "read", "then", "var", "while", "write"};

typedef enum {

	nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym,
	oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym,
	commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym,
	whilesym, dosym, callsym, constsym, varsym, procsym, writesym, readsym, elsesym

} token_type;

typedef struct Symbol {

	int kind;	// const = 1, var = 2, proc = 3
	char name[MAX_IDENT_LEN];
	token_type tokenType;  // symbol type
	int val;	// number (ASCII value)
	int level;	// L level
	int addr;	// M Address
	char *proc;

	int invalid;
	int errorCode;

} Symbol;

typedef struct LexNode {

	Symbol symbol;
	struct LexNode *next;

} LexNode;


// needed in parser.c
typedef struct Expression {
	
	int val;       // final expression value
	int s_index;   // current position in stack
	int b_index;   // current position in buffer
	char *stack;   // stack contains arithmetic operators
	char **buffer; // buffer contains postfix expression

} Expression;

typedef struct Set {

	int len; // number of symbols
	int includedTokens[50];
} Set;

typedef struct MasterSet {
	
	Set begBlock;
	Set begStatement;
	Set begCondition;
	Set begExpression;
	Set begTerm;
	Set begFactor;

} MasterSet;

typedef struct SymbolTable {

	int count;     // # var on stack
	int codeLength;// current assembly code length
	int lexLevel;  // current lexicographical level
	int u;         // unique identifier for JPC 
	int r;         // current register available
	int bp;
	char *proc[25]; // current procedure name
	
	int numErrors;
	MasterSet m;	
	Symbol *table[MAX_SYMBOL_TABLE_SIZE];
 
} SymbolTable;

typedef struct LinePair {

	char uniqueIdentifier[25]; // unique number given to a JPC instruction
	int lineNumber;            // line number that JPC goes to

} LinePair;


typedef enum {
	
	LIT = 1, RTN, LOD, STO, CAL, INC, JMP, JPC, SIO1, SIO2, SIO3, NEG, ADD, SUB,
	MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ, JMX 
} ISA;



// vm.c functions
int base(registers *reg, int *stack);
actRecords *buildAR();
int *buildStack();
void execute(registers *reg, int *stack, actRecords *ar, int print);
void fetch(instruction *code, registers *reg);
registers *initializeRegisters();
void operations(registers *reg, int *stack, actRecords *ar);
int parseInstructions(instruction *code, char *filename);
void printAssembly(instruction *code, int numInstructions);
void printRunHeader();
void printStack(registers *reg, int *stack, actRecords *ar);
instruction *readyCodeMem();
char *translateOpcode(int opcode);
void virtualMachine(char *filename, int printEx);

// lex.c functions
LexNode *createLexNode();
LexNode *destroyLexTable(LexNode *lexemeTable);
int getTokenType(char *name);
int isSpecialSymbol(char c);
void printLex(LexNode *lexemeTable, int printType);
LexNode *runLexicalAnalyzer(char *filename);

//parser.c functions
void runParser(LexNode *node);
LexNode *block(LexNode *node, SymbolTable *s);
LexNode *statement(LexNode *node, SymbolTable *s);
LexNode *condition(LexNode *node, SymbolTable *s, Expression *e);
LexNode *expression(LexNode *node, SymbolTable *s, Expression *e);
LexNode *term(LexNode *node, SymbolTable *s, Expression *e, int unaryN);
LexNode *factor(LexNode *node, SymbolTable *s, Expression *e);

//errorRecovery.c functions
int errorRecovery(LexNode *node);
LexNode *eBlock(LexNode *node, SymbolTable *s, Set *fsys);
LexNode *eStatement(LexNode *node, SymbolTable *s, Set *fsys);
LexNode *eCondition(LexNode *node, SymbolTable *s, Expression *e, Set *fsys);
LexNode *eExpression(LexNode *node, SymbolTable *s, Expression *e, Set *fsys);
LexNode *eTerm(LexNode *node, SymbolTable *s, Expression *e, Set *fsys);
LexNode *eFactor(LexNode *node, SymbolTable *s, Expression *e, Set *fsys);

#endif
