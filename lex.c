// Written by: Michael J. Pfeiffer
// Date: 2/18/17
// Last Revised: 3/22/17

LexNode *createLexNode() {

	LexNode *newNode;

	if(!(newNode = calloc(1, sizeof(LexNode)))) {

		printf("Error. Out of memory in createLexNode(). Exiting.\n");
		exit(102);
	}

	return newNode;
}

LexNode *destroyLexTable(LexNode *lexemeTable) {

	if(lexemeTable == NULL)
		return NULL;

	destroyLexTable(lexemeTable->next);
	free(lexemeTable);

	return NULL;
}

int isSpecialSymbol(char c) {

	int i;
	
	for(i = 0; i < NUM_SPECIAL_SYM; i++)
		if(c == specialSymbols[i])
			return 1;

	return 0;
}

int getTokenType(char *name) {

	if(strcmp(name, "null") == 0) return nulsym;
	if(strcmp(name, "+") == 0) return plussym;
	if(strcmp(name, "-") == 0) return minussym;
	if(strcmp(name, "*") == 0) return multsym;
	if(strcmp(name, "/") == 0) return slashsym;
	if(strcmp(name, "odd") == 0) return oddsym;
	if(strcmp(name, "=") == 0) return eqlsym;
	if(strcmp(name, "<>") == 0) return neqsym;
	if(strcmp(name, "<") == 0) return lessym;	
	if(strcmp(name, "<=") == 0) return leqsym;
	if(strcmp(name, ">") == 0) return gtrsym;
	if(strcmp(name, ">=") == 0) return geqsym;
	if(strcmp(name, "(") == 0) return lparentsym;
	if(strcmp(name, ")") == 0) return rparentsym;
	if(strcmp(name, ",") == 0) return commasym;
	if(strcmp(name, ";") == 0) return semicolonsym;
	if(strcmp(name, ".") == 0) return periodsym;
	if(strcmp(name, ":=") == 0) return becomessym;
	if(strcmp(name, "begin") == 0) return beginsym;	
	if(strcmp(name, "end") == 0) return endsym;
	if(strcmp(name, "if") == 0) return ifsym;
	if(strcmp(name, "then") == 0) return thensym;
	if(strcmp(name, "while") == 0) return whilesym;
	if(strcmp(name, "do") == 0) return dosym;
	if(strcmp(name, "call") == 0) return callsym;
	if(strcmp(name, "const") == 0) return constsym;
	if(strcmp(name, "var") == 0) return varsym;
	if(strcmp(name, "procedure") == 0) return procsym;
	if(strcmp(name, "write") == 0) return writesym;
	if(strcmp(name, "read") == 0) return readsym;
	if(strcmp(name, "else") == 0) return elsesym;

	return identsym;
}

char *getSymbolName(int tokenType) {

	int index = tokenType - 1;	

	char *tokenName[] = 
	{ "nulsym", "identsym", "numbersym", "plussym", "minussym", "multsym", "slashsym",
	  "oddsym", "eqlsym", "neqsym", "lessym", "leqsym", "gtrsym", "geqsym", "lparentsym", "rparentsym",
	  "commasym", "semicolonsym", "periodsym", "becomessym", "beginsym", "endsym", "ifsym", "thensym",
	  "whilesym", "dosym", "callsym", "constsym", "varsym", "procsym", "writesym", "readsym", "elsesym"
	};

	return tokenName[index];
}

// print type: (1) Lex Table (2) Lex List (3) Symbolic Lex List
void printLex(LexNode *headLex, int printType) {

	int tokenType;
	char *name;
	LexNode *currentNode = headLex;

	switch(printType) {
	
		case 1: printf("Lex Table:\n\n");
			break;
		case 2: printf("Lex List:\n\n");
			break;
		case 3: printf("Lex List (symbolic):\n\n");
			break; 
		default: printf("ERROR: Invalid print type in printLex().\n");
			 exit(5000);
	}

	while(currentNode->next) {
	
		tokenType = currentNode->symbol.tokenType;
		name = currentNode->symbol.name;

		switch(printType) {
	
			case 1: printf("\t%-11s %-11d\n", currentNode->symbol.name, currentNode->symbol.tokenType);
				break;
			case 2: printf("%d %s", tokenType, 
						(tokenType == 2 || tokenType == 3) ? strcat(name, " ") : "");
				break;
			case 3: printf("%s %s", getSymbolName(tokenType), 
				                (tokenType == 2 || tokenType == 3) ? strcat(name, " ") : "");
				break;
		}

		currentNode = currentNode->next;
	}

	printf("\n\n");	
}

LexNode *runLexicalAnalyzer(char *filename) {

	int numTokens = 0, currentChar;
	char bufferChar, temp;	
	
	LexNode *lexemeTable = createLexNode();
	LexNode *currentLex = lexemeTable;

	FILE *source;

	if(!(source = fopen(filename, "r"))) {

		printf("File [%s] not found. Exiting.\n", filename);
		exit(101);
	}

	// build linked list of lexical tokens
	while((bufferChar = getc(source)) != EOF){
	
		currentChar = 0;
	
		// if first character is a letter, read entire word then decipher meaning
		if(isalpha(bufferChar)) {
			
			do {
				if(currentChar >= MAX_IDENT_LEN) {

					printf("Error. Identifier exceeds maximum length. Exiting.\n");
					exit(105);
				}

				currentLex->symbol.name[currentChar++] = bufferChar;
				
			} while(isalnum(bufferChar = getc(source)));
			
			currentLex->symbol.tokenType = getTokenType(currentLex->symbol.name);

			ungetc(bufferChar, source);
		}
		
		// if first character is a digit, must be a number
		else if(isdigit(bufferChar)) {

			do {
				if(currentChar >= MAX_NUM_LEN) {

					printf("Error. Number exceeds maximum length. Exiting.\n");
					exit(104);
				}

				if(isalpha(bufferChar)) {

					//printf("Error. Identifier cannot begin with a number. Exiting.\n");
					//exit(106);
					currentLex->symbol.invalid = 1;
					currentLex->symbol.errorCode = 23;
				}
	
				currentLex->symbol.name[currentChar++] = bufferChar;

			} while(isalnum(bufferChar = getc(source))); 
			
			currentLex->symbol.tokenType = numbersym;

			ungetc(bufferChar, source);	
		}

		// if first character is special character
		else if(isSpecialSymbol(bufferChar)) {

			// special characters with only one symbol
			if(bufferChar != '<' && bufferChar != '>' && bufferChar != ':' && bufferChar != '/') {

				currentLex->symbol.name[currentChar] = bufferChar;			
			}

			// lt, lte, gt, gte, neq
			else if(bufferChar == '<' || bufferChar == '>') {
			
				currentLex->symbol.name[currentChar++] = bufferChar;

				temp = bufferChar;	
					
				if((bufferChar = getc(source)) == '='|| (temp == '<' && bufferChar == '>'))
					currentLex->symbol.name[currentChar++] = bufferChar;
				else
					ungetc(bufferChar, source);
			}
			
			// becomes symbol or invalid
			else if(bufferChar == ':') {

				currentLex->symbol.name[currentChar++] = bufferChar;
			
				if((bufferChar = getc(source)) == '=')
					currentLex->symbol.name[currentChar] = bufferChar;

				else {

					printf("Invalid symbol.  Found ':' without '='. Exiting.\n");
					exit(105);
				}
			}
		
			// slash or comments
			else if(bufferChar == '/') {

				currentLex->symbol.name[currentChar] = bufferChar;

				if((bufferChar = getc(source)) == '*') {
			
					while(!((bufferChar = getc(source)) == '*' && (temp = getc(source)) == '/')){
					
						if(bufferChar == EOF || temp == EOF) {

							printf("Error. Comment block not closed. Exiting.\n");
							exit(107);
						}
					}

					continue;
				}
				
				else 
					ungetc(bufferChar, source);

			}

			currentLex->symbol.tokenType = getTokenType(currentLex->symbol.name);
		}

		// ignore whitespace
		else if(isspace(bufferChar))
			continue;
	
		// illegal character
		else {

			printf("Error. Invalid Symbol '%c'. Exiting.\n", bufferChar);
			exit(103);
		}
		
		currentLex->next = createLexNode();
		currentLex = currentLex->next;
		numTokens++;
	}

	fclose(source);

	return lexemeTable;
}
