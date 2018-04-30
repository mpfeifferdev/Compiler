// Written by: Michael J. Pfeiffer
//             Megan Herrera
//             Yared Espinosa
// Date: 4/7/2017
// Last Revised: 4/9/17

void error(SymbolTable *s, int code) {

	char *message;

	(s->numErrors)++;
	
	switch(code) {

		case 1: message = "Use of = instead of :=";
			break;
		case 2: message = "= must be followed by a number"; 
			break;
		case 3: message = "Identifier must be followed by =";
			break;
		case 4: message = "const, var, procedure must be followed by identifier";
			break;
		case 5: message = "Semicolon or comma missing";
			break;
		case 6: message = "Incorrect symbol after procedure declaration";
			break;
		case 7: message = "Statement expected";
			break;
		case 8: message = "Incorrect symbol after statement part in block";
			break;
		case 9: message = "Period expected";
			break;
		case 10: message = "Semicolon between statements missing";
			break;
		case 11: message = "Undeclared identifier";
			break;
		case 12: message = "Assignment to constant or procedure is not allowed";
			break;
		case 13: message = "Assignment operator expected";
			break;
		case 14: message = "call must be followed by an identifier";
			break;
		case 15: message = "Call of a constant or variable is meaningless";
			break;
		case 16: message = "then expected";
			break;
		case 17: message = "Semicolon or } expected";
			break;
		case 18: message = "do expected";
			break;
		case 19: message = "Incorrect symbol following statement";
			break;
		case 20: message = "Relational operator expected";
			break;
		case 21: message = "Expression must not contain a procedure identifier";
			break;
		case 22: message = "Right parenthesis missing";
			break;
		case 23: message = "The preceding factor cannot begin with this symbol";
			break;
		case 24: message = "An expression cannot begin with this symbol";
			break;
		case 25: message = "This number is too large";
			break;
		case 26: message = "Var, Proc, or Statement Expected";
			break;
		case 27: message = "Const must be declared before Var";
			break;
		case 28: message = "Identifier already declared";
			break;
		case 29: message = "Unable to search lexigraphical levels < 0";
			break;
		case 32: message = "Program has exceeded the maximum lexicographical level";
			break;
		default: message = "Unknown error";
	}

	printf("***** Error %d: %s\n", code, message);
}

char *charToString(char c) {

	char *string = calloc(2, sizeof(char));

	string[0] = c;
	string[1] = '\0';

	return string;
}

void emit(SymbolTable *s, int op, int r, int l, int m) {

	FILE *assemblyFile = fopen(PRE_ASM, "a");

	fprintf(assemblyFile, "%d %d %d %d\n", op, r, l, m);

	(s->codeLength)++;

	fclose(assemblyFile); 
}

void amendments(int uniqueIdentifier, int lineNumber) {

	FILE *amendments = fopen(AMEND, "a");

	fprintf(amendments, "%d %d\n", uniqueIdentifier, lineNumber);
	fclose(amendments);
}

void destroyHashTable(LinePair **table) {

	int i;

	for(i = 0; i < 25; i++)
		free(table[i]);

	free(table);
}

Expression *createNewExpression() {
	
	Expression *e = calloc(1, sizeof(Expression));

	e->stack = calloc(100, sizeof(char));    // stack for arithmetic operators in infix 
	e->buffer = calloc(100, sizeof(char *)); // stores postfix expression
	
	return e;
}

void destroyExpression(Expression *e) {
	
	int i;

	if(e == NULL)
		return;

	for(i = 0; i < e->b_index; i++) 
		free(e->buffer[i]);
	
	free(e->buffer);
	free(e->stack);
	free(e);
}

int hashFunction(char *str) {

	int i, hash = 0, str_len = strlen(str);

	// Horner's rule
	for(i = 0; i < str_len; i++)
		hash = (int)str[i] + (31 * hash);
	
	hash = (hash < 0) ? (hash * -1) : hash; // adjust to positive

	return hash;
}

void finalize() {

	int opcode, r, l, lineNumber, index, i;
	char uniqueStr[25] = "";
	char m[25] = "";

	char *buffer = calloc(25, sizeof(char));

	LinePair *line; 
	LinePair **table = calloc(25, sizeof(LinePair *));

	FILE *assembly = fopen(PRE_ASM, "r");
	FILE *amendments = fopen(AMEND, "r");
	FILE *final = fopen(ASM_FINAL, "w");

	// add jump addresses to hash table
	while(fgets(buffer, 25, amendments)) {

		sscanf(buffer, "%s %d", uniqueStr, &lineNumber);
		
		line = calloc(1, sizeof(LinePair));
		strcpy(line->uniqueIdentifier, uniqueStr);
		line->lineNumber = lineNumber;

		index = hashFunction(uniqueStr) % 25;
		
		for(i = 0; table[index] != NULL && i < 1000; i++)	
			index = (index + (i * i)) % 25;
	
		if(i >= 1000) {

			printf("Unable to add LinePair to hash table\n");
			exit(10004);
		}

		table[index] = line;
	}
	// create final assembly code with accurate jump instructions
	while(fgets(buffer, 25, assembly)) {

		sscanf(buffer,"%d %d %d %s\n", &opcode, &r, &l, m);

		if(opcode == JPC || opcode == JMX) {   // jmx is special instruction for else jumps
	
			index = hashFunction(m) % 25;

			for(i = 0; table[index] != NULL && i < 1000; i++) {
		
				if(strcmp(table[index]->uniqueIdentifier, m) == 0) {
					 
					lineNumber = table[index]->lineNumber;
 					break;
				}
					
				index = (index + (i * i)) % 25;
			}	
		}
		
		else
			lineNumber = atoi(m);

		if(opcode == JMX)
			opcode = JMP;

		fprintf(final, "%d %d %d %d\n", opcode, r, l, lineNumber);
	}

	fclose(assembly);
	fclose(amendments);
	fclose(final);

	free(buffer);

	destroyHashTable(table);
}

void addSymbol(SymbolTable *s, Symbol *symbol) {

	int i;
	int index = hashFunction(symbol->name) % MAX_SYMBOL_TABLE_SIZE;


	for(i = 0; s->table[index] != NULL && i < 1000; i++)	
		index = (index + (i * i)) % MAX_SYMBOL_TABLE_SIZE;
	
	if(i >= 1000) {

		printf("Unable to add symbol to symbol table\n");
		exit(10002);
	}

	s->table[index] = symbol;
}

Symbol *getSymbol(SymbolTable *s, char *name, int level, int assertLevel) {

	int i, checkLevel, targetAcquired = 0, numFound = 0;
	int index = hashFunction(name) % MAX_SYMBOL_TABLE_SIZE;
	Symbol *temp;
	Symbol **array = calloc(25, sizeof(Symbol *));

	// build an array of Symbols matching the description
	for(i = 0; s->table[index] != NULL && i < 1000; i++) {
		
		temp = s->table[index];

		if(strcmp(temp->name, name) == 0)	
			array[numFound++] = temp;
	
		index = (index + (i * i)) % MAX_SYMBOL_TABLE_SIZE;
	}	

	temp = NULL;

	// assertLevel is typically activated during declarations
	if(numFound > 0 && assertLevel) {

		for(i = numFound - 1; i >= 0 && !targetAcquired; i--) {
			
			if(array[i]->level == level && strcmp(array[i]->proc, s->proc[level]) == 0) {

				targetAcquired = 1;
				temp = array[i];
			}			
		}
	}

	else {

		while(numFound > 0 && !targetAcquired) {
	
			if(array[--numFound]->kind != PROC && array[numFound]->level <= level) {
		
				checkLevel = level;	

				// check whether the found symbol is within the currect scope
				// s->proc contains the names of the active prcedure calls
				while(checkLevel >= 0) { 

					if((strcmp(array[numFound]->proc, s->proc[checkLevel]) == 0)
					   & (array[numFound]->level == (checkLevel)--)) {
						
						targetAcquired = 1;
						temp = array[numFound];
					}
				}
			}
			else if(array[numFound]->kind == PROC)
				temp = array[0];
		}
	}

	free(array);

	return temp; 
}

// final result stored in register in VM and possibly an Expression struct
int evaluatePostfix(SymbolTable *s, Expression *e) {

	int i, j, num1, num2, value, length, k = 0;
	int stack[100] = {0};
	char **buffer;
	Symbol *temp;
	
	length = e->b_index;
	buffer = e->buffer;	
	
	(s->r)++;

	// i is index in stack, j is index in buffer
	for(i = 0, j = 0; j < length; i++, j++) {


		if(isdigit(buffer[j][0])) {

			stack[i] = atoi(buffer[j]);
			emit(s, LIT, s->r + i, 0, atoi(buffer[j])); // LIT R, L, M
			
		}
		
		else if(isalpha(buffer[j][0])) {
		
			if((temp = getSymbol(s, buffer[j], s->lexLevel, 0)) == NULL) 
				error(s, 11); // undeclared identifier

			stack[i] = temp->val;

			if(temp->kind == CONST)
				emit(s, LIT, s->r + i, 0, temp->val);	
			else
				emit(s, LOD, s->r + i, s->lexLevel - temp->level, temp->addr);	
		}

		// else arithmetic operator
		else {
			if(j - 1 < 0 || (i - 2 < 0 && buffer[j][0] != '!') || i - 1 < 0) {

				printf("Postfix incorrect formatting. Index < 0\n");
				exit(1000);
			}

			num1 = stack[i - 2];
			num2 = stack[i - 1];			

			switch(buffer[j][0]) {

				case '+' : value = num1 + num2;
					   emit(s, ADD, s->r + i - 2, s->r + i - 2, s->r + i - 1);
					break;
				case '-' : value = num1 - num2;
				           emit(s, SUB, s->r + i - 2, s->r + i - 2, s->r + i - 1);
					break;
				case '*' : value = num1 * num2;
					   emit(s, MUL, s->r + i - 2, s->r + i - 2, s->r + i - 1);
					break;
				case '/' : value = num1 / num2;
					   emit(s, DIV, s->r + i - 2, s->r + i - 2, s->r + i - 1);
					break;
				case '!' : value *= -1;
				           emit(s, NEG, s->r + i - 1, 0, 0);
					break;
				default: printf("Invalid operator in postfix\n");
					 exit(1001);
			}  

			if(buffer[j][0] != '!')
				stack[i -= 2] = value; // decrement register file/stack pointer
			else
				stack[i -= 1] = value;
		}
	}

	return stack[0];
}

LexNode *getToken(LexNode *currentLex) {

	return currentLex->next;
}

int isRelational(int type) {

	switch(type) {
	
		case eqlsym:
		case neqsym:
		case lessym:
		case leqsym:
		case gtrsym:
		case geqsym:
			return 1;
	}

	return 0;
}


LexNode *factor(LexNode *currentLex, SymbolTable *s, Expression *e) {

	char *newBufVal;

	newBufVal = calloc(strlen(currentLex->symbol.name) + 1, sizeof(char));
	strcpy(newBufVal, currentLex->symbol.name);
	
	if(currentLex->symbol.tokenType == identsym) {
			
		if(currentLex->symbol.kind == PROC)
			error(s, 21); // Expression must not contain a procedure ident
		
		e->buffer[e->b_index++] = newBufVal; // add var/const name to buffer

		currentLex = getToken(currentLex);
	}

	else if(currentLex->symbol.tokenType == numbersym) {

		e->buffer[e->b_index++] = newBufVal; // add number to buffer
		
		currentLex = getToken(currentLex);
	}

	else if(currentLex->symbol.tokenType == lparentsym) {

		free(newBufVal);
		
		e->stack[e->s_index++] = currentLex->symbol.name[0]; // add left paren to stack

		currentLex = getToken(currentLex);
		currentLex = expression(currentLex, s, e);

		if(currentLex->symbol.tokenType != rparentsym)
			error(s, 22); // right parenthesis missing

		if(e->stack[e->s_index - 1] == '(')
			e->s_index--; // get right of left parenthesis

		currentLex = getToken(currentLex);
	}

	else
		error(s, 7); // statement expected

	return currentLex;
}

int popConditionsStatus(Expression *e, int factor) {

	// plus/minus logic
	if(e->s_index && e->stack[e->s_index - 1] != '(') {
		if(!factor)
			return 1;
	}

 	else
		return 0;
 
	// mult/div logic
	if(e->s_index && e->stack[e->s_index - 1] != '+' && e->stack[e->s_index - 1] != '-')
		return 1;
	
	return 0;
}

void addNegate(Expression *e) {

	char *negateCommand = calloc(2, sizeof(char));

	strcpy(negateCommand, "!");
	e->buffer[e->b_index++] = negateCommand;
}
void popStack(Expression *e, int factor) {
	
	while(popConditionsStatus(e, factor)) 
		e->buffer[e->b_index++] = charToString(e->stack[--e->s_index]);
}

void expressionLogic(LexNode *currentLex, Expression *e, int factor) {

	popStack(e, factor);	
	e->stack[e->s_index++] = currentLex->symbol.name[0]; // add (+)(-)(/)(*) to stack
}

LexNode *term(LexNode *currentLex, SymbolTable *s, Expression *e, int unaryN) {

	currentLex = factor(currentLex, s, e);

	if(unaryN)
		addNegate(e);

	while(currentLex->symbol.tokenType == multsym || currentLex->symbol.tokenType == slashsym) {

		expressionLogic(currentLex, e, 1);
	
		currentLex = getToken(currentLex);
		currentLex = factor(currentLex, s, e);
	}

	return currentLex;
}

LexNode *expression(LexNode *currentLex, SymbolTable *s, Expression *e) {
	
	int tokenType, unaryN = 0; // (-) 

	tokenType = currentLex->symbol.tokenType;

	if(!(tokenType == plussym || tokenType == minussym || tokenType == lparentsym || 
	     tokenType == numbersym || tokenType == identsym))
		error(s, 24); // expression cannot begin with this symbol

	if((unaryN = (tokenType == minussym)) || tokenType == plussym)	
		currentLex = getToken(currentLex);

	currentLex = term(currentLex, s, e, unaryN);

	while(currentLex->symbol.tokenType == plussym || currentLex->symbol.tokenType == minussym) {

		expressionLogic(currentLex, e, 0);	
		currentLex = getToken(currentLex);
		currentLex = term(currentLex, s, e, 0); // negate only applies to first term
	}

	popStack(e, 0);

	if(e->s_index == 0)
		e->val = evaluatePostfix(s, e); 

	return currentLex;
}

LexNode *condition(LexNode *currentLex, SymbolTable *s, Expression *e1) {

	int values[2];
	int relType;	

	Expression *e2 = createNewExpression();
	
	if(currentLex->symbol.tokenType == oddsym) {

		currentLex = getToken(currentLex);
		currentLex = expression(currentLex, s, e1);

		emit(s, ODD, s->r, 0, 0); 
	}

	else {
		currentLex = expression(currentLex, s, e1);

		if(!isRelational(relType = currentLex->symbol.tokenType))
			error(s, 20); // relational operator expected
		
		currentLex = getToken(currentLex); 
		currentLex = expression(currentLex, s, e2);

		// final result stored in reg	
		switch(relType) {
			
			case 9:  emit(s, EQL, s->r - 1, s->r - 1, s->r); // EQL R, R, R 
				break;
			case 10: emit(s, NEQ, s->r - 1, s->r - 1, s->r); // NEQ
				break;
			case 11: emit(s, LSS, s->r - 1, s->r - 1, s->r); // LSS
				break;
			case 12: emit(s, LEQ, s->r - 1, s->r - 1, s->r); // LEQ
				break;
			case 13: emit(s, GTR, s->r - 1, s->r - 1, s->r); // GTR
				break;
			case 14: emit(s, GEQ, s->r - 1, s->r - 1, s->r); // GEQ
				break;
			default: error(s, 20); // relational operator expected
		}

		(s->r)--;
	}
	
	destroyExpression(e2);

	return currentLex;
}

LexNode *identStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	Symbol *temp;

	if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL)  	
		error(s, 11); // undeclared identifier	

	else if(temp->kind != VAR)
		error(s, 12); // assignment to constant or proc not allowed

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType == eqlsym)
		error(s, 1); // Use of = instead of :=

	if(currentLex->symbol.tokenType != becomessym)
		error(s, 13); // assignment operator expected

	currentLex = getToken(currentLex);
	currentLex = expression(currentLex, s, e);

	// update symbol table
	temp->val = e->val;

	emit(s, STO, s->r, s->lexLevel - temp->level, temp->addr); // postfix val goes to var addr in mem
		
	(s->r)--;
		
	return currentLex;
}

LexNode *callStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	int i = 0;
	Symbol *temp;

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType != identsym)
		error(s, 14); // call must be followed by an identifier

	if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL) 
		error(s, 11); // undeclared identifier

	else if(temp->kind != PROC)
		error(s, 15); // call of a constant or variable is meaningless

	emit(s, CAL, 0, s->lexLevel - ((temp->level - 1) < 0 ? 0 : temp->level - 1), temp->addr); 

	currentLex = getToken(currentLex);
	
	return currentLex;
}

LexNode *beginStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	currentLex = getToken(currentLex);
	currentLex = statement(currentLex, s);

	while(currentLex->symbol.tokenType == semicolonsym) {

		currentLex = getToken(currentLex);
		currentLex = statement(currentLex, s); // end is not statement, so does nothing 
	}

	if(currentLex->symbol.tokenType != endsym) 
		error(s, 8); // incorrect symbol after statement
		
	currentLex = getToken(currentLex);
	
	return currentLex;
}

LexNode *ifStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	int jpcIdent, jmpIdent = 0;

	currentLex = getToken(currentLex);
	currentLex = condition(currentLex, s, e);
	
	jpcIdent = (s->u)++;
	emit(s, JPC, s->r--, 0, jpcIdent); // conditional jump

	if(currentLex->symbol.tokenType != thensym)
		error(s, 16); // then expected
		
	currentLex = getToken(currentLex);
	currentLex = statement(currentLex, s);

	if(currentLex->symbol.tokenType == elsesym) {

		jmpIdent = (s->u)++;

		emit(s, JMX, 0, 0, jmpIdent); // at end of if true, jump past else

		amendments(jpcIdent, s->codeLength); // if is false, now inside else
		
		currentLex = getToken(currentLex);
		currentLex = statement(currentLex, s);
		
		amendments(jmpIdent, s->codeLength); // now past else statement
	}
	
	else
		amendments(jpcIdent, s->codeLength); // without else statement
	
	return currentLex;
}

LexNode *whileStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	int storedInstruction;
	int uniqueIdentifier;

	currentLex = getToken(currentLex);
	
	storedInstruction = s->codeLength; // store next line number

	currentLex = condition(currentLex, s, e);

	uniqueIdentifier = s->u;
	emit(s, JPC, s->r--, 0, (s->u)++);

	if(currentLex->symbol.tokenType != dosym)
		error(s, 18); // do expected
	
	currentLex = getToken(currentLex);
	currentLex = statement(currentLex, s);

	emit(s, JMP, 0, 0, storedInstruction); // return to conditional in while

	amendments(uniqueIdentifier, s->codeLength);
	
	return currentLex;
}

LexNode *readStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	Symbol *temp;
	FILE *inputFile;

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType != identsym) 
		error(s, 26); // incorrect symbol after Read
		
	if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) != NULL 
	   && temp->kind == VAR) {
		
		inputFile = fopen(VM_IN, "a");

		printf("Input (%s): ", temp->name);
		scanf("%d", &temp->val);

		fprintf(inputFile, "%d\n", temp->val);
		fclose(inputFile);

		emit(s, SIO2, s->r, 0, 2); // read to reg
		emit(s, STO, s->r, s->lexLevel - temp->level, temp->addr); // save to mem location
	}

	else if(temp == NULL)
		error(s, 11); // undeclared identifier	

	else if(temp->kind != VAR)
		error(s, 12); // assignment to constant or proc not allowed

	currentLex = getToken(currentLex);	

	return currentLex;
}

LexNode *writeStatement(LexNode *currentLex, SymbolTable *s, Expression *e) {

	Symbol *temp;

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType != identsym)
		error(s, 27); // incorrect symbol after Write

	if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) != NULL) {

		emit(s, LOD, s->r, s->lexLevel - temp->level, temp->addr); // -1 accounts for base pointer
		emit(s, SIO1, s->r, 0, 1);
	}

	else if(temp == NULL)
		error(s, 11); // undeclared identifier

	else if(temp->kind != VAR)
		error(s, 12); // assignment to constant or proc not allowed

	currentLex = getToken(currentLex);

	return currentLex;
}

LexNode *statement(LexNode *currentLex, SymbolTable *s) {

	Expression *e = createNewExpression();	

	if(currentLex->symbol.tokenType == identsym)
		currentLex = identStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == callsym) 
		currentLex = callStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == beginsym)
		currentLex = beginStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == ifsym) 
		currentLex = ifStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == whilesym) 
		currentLex = whileStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == readsym) 
		currentLex = readStatement(currentLex, s, e);
	
	else if(currentLex->symbol.tokenType == writesym) 
		currentLex = writeStatement(currentLex, s, e);

	else if(currentLex->symbol.tokenType == semicolonsym)
		; // empty string
		
	else if(currentLex->symbol.tokenType == endsym)
		; // empty string 
	else { 
		error(s, 7); // statement expected
	}
	destroyExpression(e);

	return currentLex;
}

LexNode *constDeclaration(LexNode *currentLex, SymbolTable *s) {
	
	Symbol *temp;

	do {
		currentLex = getToken(currentLex);
		temp = &(currentLex->symbol); // store symbol location
		
		if(temp->tokenType != identsym) {
			error(s, 4); // const must be followeed by an identifier
		}
		if(getSymbol(s, temp->name, s->lexLevel, 1) != NULL)
			error(s, 28); // identifier already declared
		
		currentLex = getToken(currentLex);

		if(currentLex->symbol.tokenType == becomessym)
			error(s, 1); // use of := instead of =

		else if(currentLex->symbol.tokenType != eqlsym)
			error(s, 3); // ident must be followed by =

		currentLex = getToken(currentLex);

		if(currentLex->symbol.tokenType != numbersym)
			error(s, 2); // = must be followed by a number

		temp->kind = CONST;
		temp->level = s->lexLevel;
		temp->val = atoi(currentLex->symbol.name);
		temp->proc = s->proc[s->lexLevel];

		addSymbol(s, temp);  // add to symbol table

		currentLex = getToken(currentLex);	

	} while(currentLex->symbol.tokenType == commasym);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5); // semicolon or comma missing

	else
		currentLex = getToken(currentLex);

	return currentLex;
}

LexNode *varDeclaration(LexNode *currentLex, SymbolTable *s) {

	Symbol *temp;

	do {
		currentLex = getToken(currentLex);
		temp = &(currentLex->symbol);
		
		if(temp->tokenType != identsym) {
			error(s, 4); // var must be followed by an identifier
		}
		if(getSymbol(s, temp->name, s->lexLevel, 1) != NULL)
			error(s, 28); // identifier already declared

		temp->kind = VAR;
		temp->level = s->lexLevel;
		temp->addr = s->count + 4; // gets next slot in memory
		temp->proc = s->proc[s->lexLevel];

		addSymbol(s, temp); // add to symbol table
		(s->count)++;	

		currentLex = getToken(currentLex);

	} while(currentLex->symbol.tokenType == commasym);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5); // semicolon or comma missing
		
	currentLex = getToken(currentLex);
	
	return currentLex;
}

LexNode *procDeclaration(LexNode *currentLex, SymbolTable *s) {
	
	Symbol *temp;

	currentLex = getToken(currentLex);
	temp = &(currentLex->symbol);

	if(temp->tokenType != identsym)
		error(s, 4); // procedure must be followed by an identifier

	if(getSymbol(s, temp->name, ++(s->lexLevel), 1) != NULL)
		error(s, 28); // identifier already declared

	temp->kind = PROC;
	temp->level = s->lexLevel;         // L
	temp->addr = s->codeLength;        // M 
	s->proc[s->lexLevel] = temp->name; // track procedure name at each level

	addSymbol(s, temp);	

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 6); // incorrect symbol after proc declaration
		
	currentLex = getToken(currentLex);
	currentLex = block(currentLex, s);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5);

	emit(s, RTN, 0, 0, 0);

	(s->lexLevel)--; // revert state

	currentLex = getToken(currentLex);

	return currentLex;
}

LexNode *block(LexNode *currentLex, SymbolTable *s) {

	int prevCount = s->count, prevBP, jmpLoc, areProcedures = 0;  // save state

	if(s->lexLevel > MAX_LEXI_LEVELS)
		error(s, 32); 

	s->count = 0;

	if(currentLex->symbol.tokenType == constsym) 
		currentLex = constDeclaration(currentLex, s);

	if(currentLex->symbol.tokenType == varsym) 
		currentLex = varDeclaration(currentLex, s);
	
	emit(s, INC, 0, 0, s->count + 4); // increment stack based on number of variables
	
	if(currentLex->symbol.tokenType == procsym) {
		
		areProcedures = 1;
		emit(s, JMX, 0, 0, jmpLoc = (s->u)++);
	}
	
	while(currentLex->symbol.tokenType == procsym) {

		s->bp += s->count;
		prevBP = s->bp;
		currentLex = procDeclaration(currentLex, s);
		s->bp = prevBP;
	}

	if(areProcedures)
		amendments(jmpLoc, s->codeLength);

	currentLex = statement(currentLex, s);

	
	s->count = prevCount; // revert state
	s->bp = prevBP;

	return currentLex;
}

void runParser(LexNode *currentLex) {

	LexNode *head = currentLex;
	SymbolTable *s = calloc(1, sizeof(SymbolTable));	

	s->proc[0] = "";

	FILE *assemblyFile = fopen(PRE_ASM, "w"); // overwrite/create new file
	FILE *amendmentsFile = fopen(AMEND, "w");
	FILE *inputFile = fopen(VM_IN, "w");
	FILE *outputFile = fopen(VM_OUT, "w");
	
	fclose(assemblyFile);
	fclose(amendmentsFile); 
	fclose(inputFile); 
	fclose(outputFile);
            
	currentLex = block(currentLex, s);

	if(currentLex->symbol.tokenType != periodsym)
		error(s, 9);  // period expected	

	emit(s, SIO3, 0, 0, 3); // halt program

	finalize(); // builds final assembly file with jump locations

	free(s);
}
