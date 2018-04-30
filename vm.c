// Written by: Michael J. Pfeiffer
// NID:        mi957047
// Date:       1/29/17

/*		  ISA
	01 - LIT	13 - ADD
	02 - RTN	14 - SUB
	03 - LOD	15 - MUL
	04 - STO	16 - DIV
	05 - CAL	17 - ODD
	06 - INC	18 - MOD
	07 - JMP	19 - EQL
	08 - JPC	20 - NEQ
	09 - SIO1	21 - LSS
	10 - SIO2	22 - LEQ
	11 - SIO3	23 - GTR
	12 - NEG	24 - GEQ
*/

int parseInstructions(instruction *code, char *fileName) {
	
	int i = 0;
	char *readInstruction = calloc(MAX_INSTRUCTION_LEN, sizeof(char));
	FILE *fp = fopen(fileName, "r");

	if(fp == NULL) {
		
		printf("Error. Unable to locate file %s.\n", fileName);
		free(code);
		exit(1);
	}

	while(fgets(readInstruction, MAX_INSTRUCTION_LEN, fp)) {

		if(i >= MAX_CODE_LENGTH) {

			printf("Program is too large. MAX_CODE_LENGTH is %d. Exiting...\n", MAX_CODE_LENGTH);
			exit(107);
		}		

		sscanf(readInstruction, "%d %d %d %d", &(code[i].op), &(code[i].r),
                                                       &(code[i].L), &(code[i].M));
		i++;
	}
	
	free(readInstruction);
	fclose(fp);

	return i;
}

// returns the desired base L levels down
int base(registers *reg, int *stack) {

	int desiredBase = reg->bp, lexLevel = reg->ir.L;

	if(lexLevel > MAX_LEXI_LEVELS) {

		printf("Action goes beyond MAX_LEXI_LEVELS: %d. Exiting...\n", MAX_LEXI_LEVELS);
		exit(104);
	}

	while(lexLevel > 0) {

		desiredBase = stack[desiredBase + 1];
		lexLevel--;
	}

	if(desiredBase < 0) {

		printf("Activation record %d level(s) down does not exist. Exiting...\n", reg->ir.L);
		exit(105);
	}

	return desiredBase;
}

void operations(registers *reg, int *stack, actRecords *ar) {

	int i;
	char c;
	char buffer[25];
	FILE *output, *input;

	switch(reg->ir.op) {
		// LIT
		case 1: reg->rf[reg->ir.r] = reg->ir.M; 
			break;
		// RTN
		case 2: reg->sp = reg->bp - 1;
			reg->bp = stack[reg->sp + 3];
			reg->pc = stack[reg->sp + 4];
			ar->size--;
			break;
		// LOD
		case 3: reg->rf[reg->ir.r] = stack[base(reg, stack) + reg->ir.M];
			break;
		// STO
		case 4: stack[base(reg, stack) + reg->ir.M] = reg->rf[reg->ir.r];
			break;
		// CAL
		case 5: 
			stack[reg->sp + 1] = 0;
			stack[reg->sp + 2] = base(reg, stack);
			stack[reg->sp + 3] = reg->bp;
			stack[reg->sp + 4] = reg->pc;
			reg->bp = reg->sp + 1;
			reg->pc = reg->ir.M;
			ar->bases[ar->size] = reg->bp;
			ar->size++;			
			break;
		// INC
		case 6: reg->sp = reg->sp + reg->ir.M;
			break;
		// JMP
		case 7: reg->pc = reg->ir.M; 
			break;
		// JPC
		case 8: reg->pc = (reg->rf[reg->ir.r] == 0) ? reg->ir.M : reg->pc;
			break;
		// SIO
		case 9:
		case 10:
		case 11:
			switch(reg->ir.M) {
			
				case 1: output = fopen(VM_OUT, "a");
					fprintf(output, "%d\n", reg->rf[reg->ir.r]);
					fclose(output);
					break;
				case 2: 
					input = fopen(VM_IN, "r");
					
					for(i = 0; i < reg->inputCount; i++) 
						fgets(buffer, 25, input);					
				
					fscanf(input, "%d", &(reg->rf[reg->ir.r]));

					reg->inputCount++;
					fclose(input);
					break;
				case 3: reg->haltFlag = 1;
					break;
				default:
					printf("Invalid input in SIO().\n");
			}
			break;
		// NEG
		case 12: reg->rf[reg->ir.r] = reg->rf[reg->ir.r] * -1;
			break;
		// ADD
		case 13: reg->rf[reg->ir.r] = reg->rf[reg->ir.L] + reg->rf[reg->ir.M];
			break;
		// SUB
		case 14: reg->rf[reg->ir.r] = reg->rf[reg->ir.L] - reg->rf[reg->ir.M];
			break;
		// MUL
		case 15: reg->rf[reg->ir.r] = reg->rf[reg->ir.L] * reg->rf[reg->ir.M];
			break;
		// DIV
		case 16: if(reg->rf[reg->ir.M] != 0)
				reg->rf[reg->ir.r] = reg->rf[reg->ir.L] / reg->rf[reg->ir.M];
			break;
		// ODD
		case 17: reg->rf[reg->ir.r] = reg->rf[reg->ir.r] % 2;
			break;
		// MOD
		case 18: reg->rf[reg->ir.r] = reg->rf[reg->ir.L] % reg->rf[reg->ir.M];
			break;
		// EQL
		case 19: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] == reg->rf[reg->ir.M]);
			break;
		// NEQ
		case 20: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] != reg->rf[reg->ir.M]);
			break;
		// LSS
		case 21: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] < reg->rf[reg->ir.M]);
			break;
		// LEQ
		case 22: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] <= reg->rf[reg->ir.M]);
			break;
		// GTR
		case 23: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] > reg->rf[reg->ir.M]);
			break;
		// GEQ
		case 24: reg->rf[reg->ir.r] = (reg->rf[reg->ir.L] >= reg->rf[reg->ir.M]);
			break;
		default:
			printf("Invalid opcode: %d.\n", reg->ir.op);
	}

	if(reg->sp >= MAX_STACK_HEIGHT) {

		printf("Stack pointer has gone beyond the MAX_STACK_HEIGHT %d. Exiting...\n", MAX_STACK_HEIGHT);
		exit(103);
	}
}

char *translateOpcode(int opcode) {

	char *ops[24] = {"lit", "rtn", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "sio", "sio",
			 "neg", "add", "sub", "mul", "div", "odd", "mod", "eql", "neq", "lss", "leq", 
			 "gtr", "geq"};

	return ops[opcode - 1];
}

void printAssembly(instruction *code, int numInstructions) {

	int i;

	printf("\nInterpreting Assembly Instructions...\n\n");
	printf("%4s %4s %4s %4s %4s\n", "Line", "OP", "R", "L", "M");

	for(i = 0; i < numInstructions; i++) {

		printf("%4d %4s %4d %4d %4d\n", i, translateOpcode(code[i].op), 
						code[i].r, code[i].L, code[i].M);
	}
}

void printStack(registers *reg, int *stack, actRecords *ar) {

	int i, currentAR = 0;

	printf("%4s %4d %4d %4d %3d %3d %4d  ", translateOpcode(reg->ir.op), 
				 reg->ir.r, reg->ir.L, reg->ir.M, reg->pc, reg->bp, reg->sp);

	if(reg->sp == 0)
		printf("\n");
	else {	
		for(i = 1; i <= reg->sp; i++) {
			
			if(i == ar->bases[currentAR]) {
				
				printf("|");
				currentAR++;
			}

			printf("%d%c", stack[i], (i == reg->sp) ? '\n' : ' ');
		}
	}
}

void fetch(instruction *code, registers *reg) {

	reg->ir = code[reg->pc];
	reg->pc = reg->pc + 1;
}

void execute(registers *reg, int *stack, actRecords *ar, int print) {
	
	int prevPC = reg->pc;	

	operations(reg, stack, ar);
	
	if(print) {

		printf("%4d ", prevPC - 1);
		printStack(reg, stack, ar);
	}
}

void printRunHeader() {

	printf("\nRunning program...\n\n");
	printf("%4s %4s %4s %4s %4s %3s %3s %4s\n", "Line", "OP", "R", "L", "M", "PC", "BP", "SP");
}

instruction *readyCodeMem() {

	instruction *code;

	if(!(code = calloc(MAX_CODE_LENGTH, sizeof(instruction)))) {
	
		printf("Out of memory in readyCodeMem(). Exiting.\n");
		exit(99);
	}

	return code;
}

int *buildStack() {

	int *stack;

	if(!(stack = calloc(MAX_STACK_HEIGHT, sizeof(int)))) {

		printf("Out of memory in buildStack(). Exiting.\n");
		exit(100);
	}

	return stack;
}

registers *initializeRegisters() {

	registers *reg;

	if(!(reg = calloc(1, sizeof(registers)))){
		
		printf("Out of memory in initializeRegisters(). Exiting.\n");
		exit(101);
	}

	reg->bp = 1;

	return reg;
}

actRecords *buildAR() {

	actRecords *ar;

	if(!(ar = calloc(1, sizeof(actRecords)))) {
	
		printf("Out of memory in buildAR(). Exiting.\n");
		exit(102);
	}

	return ar;
}

void runProgram(instruction *code, registers *reg, int *stack, actRecords *ar, int printEx) {

	while(!(reg->haltFlag) && (reg->bp > 0)) {

		fetch(code, reg);
		execute(reg, stack, ar, printEx);
	}

	if(printEx)
		printf("\n");
}

void printTextFile(char *filename) {

	char c;
	FILE *f = fopen(filename, "r");

	while((c = getc(f)) != EOF)
		putchar(c);

	printf("\n");
	fclose(f);
}
// takes assembly file, outputs execution
void virtualMachine(char *filename, int printEx) {
	
	int numInstructions, *stack;
	registers *reg;
	instruction *code;
	actRecords *ar;

	code = readyCodeMem();
	numInstructions = parseInstructions(code, filename);
	
	// initialize Registers and Memory
	stack = buildStack();
	reg = initializeRegisters();
	ar = buildAR();

	if(printEx) {
 	
		printAssembly(code, numInstructions);
		printRunHeader();
	}
	
	runProgram(code, reg, stack, ar, printEx);
	
	free(stack);
	free(reg);
	free(code);
	free(ar);
}
