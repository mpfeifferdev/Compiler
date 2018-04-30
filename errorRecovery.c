// Written by:  Michael J. Pfeiffer
//	        Megan Herrera
//		Yared Espinosa

// Date:	4/16/17


int contains(int *array, int n, int key) {

	int i;

	for(i = 0; i < n; i++)
		if(array[i] == key)
			return 1;

	return 0;
}

LexNode *test(LexNode *currentLex, SymbolTable *s, Set *a, Set *b, int errorCode) {

	int i, found = 0;

	
	if(!contains(a->includedTokens, a->len, currentLex->symbol.tokenType)) {

		error(s, errorCode);
		
		while(!found && currentLex->next != NULL) {

			found = (contains(a->includedTokens, a->len, currentLex->symbol.tokenType)
                                || contains(b->includedTokens, b->len, currentLex->symbol.tokenType));

			if(!found)
				currentLex = getToken(currentLex); 
		}

		if(currentLex->next == NULL) {

			printf("****Error: reached end of file in error recovery.\n");
			exit(666);
		}
	}

	return currentLex;
}

LexNode *eFactor(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Set amendedFsys, emptySet;
	Symbol *temp;

	emptySet.len = 0;

	if(currentLex->symbol.invalid)
		error(s, currentLex->symbol.errorCode);

	if(currentLex->symbol.tokenType == identsym) {
			
		if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL)
			error(s, 11);

		else if(temp->kind == PROC)	
			error(s, 21); // Expression must not contain a procedure ident

		currentLex = getToken(currentLex);
	}

	else if(currentLex->symbol.tokenType == numbersym)
		currentLex = getToken(currentLex);

	else if(currentLex->symbol.tokenType == lparentsym) {

		amendedFsys.len = 0;
		amendedFsys.includedTokens[amendedFsys.len++] = periodsym;
		amendedFsys.includedTokens[amendedFsys.len++] = semicolonsym;
		amendedFsys.includedTokens[amendedFsys.len++] = rparentsym;
		amendedFsys.includedTokens[amendedFsys.len++] = endsym;
		amendedFsys.includedTokens[amendedFsys.len++] = thensym;
		amendedFsys.includedTokens[amendedFsys.len++] = dosym;

		currentLex = getToken(currentLex);
		currentLex = eExpression(currentLex, s, e, &amendedFsys);

		currentLex = test(currentLex, s, &amendedFsys, &emptySet, 23); 
		
		if(currentLex->symbol.tokenType != rparentsym)
			error(s, 22); // right parenthesis missing
		else
			currentLex = getToken(currentLex);
	}
	
	return currentLex;
}

LexNode *eTerm(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Set amendedFsys = *fsys;

	if(!contains(amendedFsys.includedTokens, amendedFsys.len, multsym))
		amendedFsys.includedTokens[amendedFsys.len++] = multsym;
	if(!contains(amendedFsys.includedTokens, amendedFsys.len, slashsym))
		amendedFsys.includedTokens[amendedFsys.len++] = slashsym;

	currentLex = eFactor(currentLex, s, e, &amendedFsys);

	while(currentLex->symbol.tokenType == multsym || currentLex->symbol.tokenType == slashsym) {

		currentLex = getToken(currentLex);
		currentLex = eFactor(currentLex, s, e, &amendedFsys);
	}

	return currentLex;
}

LexNode *eExpression(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {
	
	Set amendedFsys = *fsys;
	
	if(!contains(amendedFsys.includedTokens, amendedFsys.len, plussym))
		amendedFsys.includedTokens[amendedFsys.len++] = plussym;
	if(!contains(amendedFsys.includedTokens, amendedFsys.len, minussym))
		amendedFsys.includedTokens[amendedFsys.len++] = minussym;
	
	currentLex = test(currentLex, s, &(s->m.begExpression), &amendedFsys, 24);
	
	if(currentLex->symbol.tokenType == plussym || currentLex->symbol.tokenType == minussym)
		currentLex = getToken(currentLex);

	currentLex = eTerm(currentLex, s, e, &amendedFsys);

	while(currentLex->symbol.tokenType == plussym || currentLex->symbol.tokenType == minussym) {

		currentLex = getToken(currentLex);
		currentLex = eTerm(currentLex, s, e, &amendedFsys);
	}
	
	return currentLex;
}

LexNode *eCondition(LexNode *currentLex, SymbolTable *s, Expression *e1, Set *fsys) {

	Expression *e2 = createNewExpression();
	Set amendedFsys;

	if(currentLex->symbol.tokenType == oddsym) {

		currentLex = getToken(currentLex);
		currentLex = eExpression(currentLex, s, e1, fsys);
	}

	else {
		amendedFsys = *fsys;

		if(!contains(amendedFsys.includedTokens, amendedFsys.len, eqlsym)) {
			
			amendedFsys.includedTokens[amendedFsys.len++] = eqlsym;
			amendedFsys.includedTokens[amendedFsys.len++] = neqsym;
			amendedFsys.includedTokens[amendedFsys.len++] = lessym;
			amendedFsys.includedTokens[amendedFsys.len++] = gtrsym;
			amendedFsys.includedTokens[amendedFsys.len++] = leqsym;
			amendedFsys.includedTokens[amendedFsys.len++] = geqsym;
		}
	
		currentLex = eExpression(currentLex, s, e1, &amendedFsys);

		if(!isRelational(currentLex->symbol.tokenType))
			error(s, 20); // relational operator expected
		else
			currentLex = getToken(currentLex); 

		currentLex = eExpression(currentLex, s, e2, fsys);
	}
	
	destroyExpression(e2);

	return currentLex;
}
LexNode *eIdentStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Symbol *temp;
	Set emptySet;

	emptySet.len = 0;

	if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL)  	
		currentLex = test(currentLex, s, fsys, &emptySet, 11);

	else if(temp->kind == VAR) {

		currentLex = getToken(currentLex);

		if(currentLex->symbol.tokenType == eqlsym) {

			error(s, 1); // Use of = instead of :=
			currentLex = getToken(currentLex);
		}
		else if(currentLex->symbol.tokenType != becomessym)
			error(s, 13); // assignment operator expected
		else
			currentLex = getToken(currentLex);

		currentLex = eExpression(currentLex, s, e, fsys);

		//currentLex = test(currentLex, s, &(s->m.begStatement), fsys, 10); 
	}
	
	else
		currentLex = test(currentLex, s, fsys, &emptySet, 12);

	return currentLex;
}

LexNode *eCallStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Symbol *temp;
	Set emptySet;

	emptySet.len = 0;

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType == identsym) {

		if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL) 
			error(s, 11); // undeclared identifier

		else {
			if(temp->kind == PROC)
				currentLex = getToken(currentLex);
			else
				currentLex = test(currentLex, s, fsys, &emptySet, 15);
				//error(s, 15); // call of a constant or variable is meaningless
		}
	}

	else
		currentLex = test(currentLex, s, fsys, &emptySet, 14);

	return currentLex;
}

LexNode *eBeginStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Set emptySet;

	emptySet.len = 0;

	currentLex = getToken(currentLex);
	currentLex = eStatement(currentLex, s, fsys);

	while(currentLex->symbol.tokenType == semicolonsym) {

		if(currentLex->symbol.tokenType == semicolonsym)
			currentLex = getToken(currentLex);
		currentLex = eStatement(currentLex, s, fsys); // end is not statement, so does nothing 
	}

	if(currentLex->symbol.tokenType == endsym) 
		currentLex = getToken(currentLex);
	else
		currentLex = test(currentLex, s, fsys, &emptySet, 17);
	
	return currentLex;
}

LexNode *eIfStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Set condFsys, amendedFsys;

	condFsys = amendedFsys = *fsys;

	if(!contains(condFsys.includedTokens, condFsys.len, thensym))
		condFsys.includedTokens[condFsys.len++] = thensym;
	if(!contains(condFsys.includedTokens, condFsys.len, dosym))
		condFsys.includedTokens[condFsys.len++] = dosym;	
	if(!contains(amendedFsys.includedTokens, amendedFsys.len, elsesym))
		amendedFsys.includedTokens[amendedFsys.len++] = elsesym;

	currentLex = getToken(currentLex);
		
	currentLex = eCondition(currentLex, s, e, &condFsys);
	
	if(currentLex->symbol.tokenType != thensym)
		error(s, 16); // then expected
	else	
		currentLex = getToken(currentLex);
	
	currentLex = eStatement(currentLex, s, &amendedFsys);

	if(currentLex->symbol.tokenType == elsesym) {

		currentLex = getToken(currentLex);
		currentLex = eStatement(currentLex, s, fsys);		
	}
	
	return currentLex;
}

LexNode *eWhileStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Set condFsys = *fsys;
	
	if(!contains(condFsys.includedTokens, condFsys.len, thensym))
		condFsys.includedTokens[condFsys.len++] = thensym;
	if(!contains(condFsys.includedTokens, condFsys.len, dosym))
		condFsys.includedTokens[condFsys.len++] = dosym;	

	currentLex = getToken(currentLex);

	currentLex = eCondition(currentLex, s, e, &condFsys);
	
	if(currentLex->symbol.tokenType != dosym)
		error(s, 18); // do expected
	else
		currentLex = getToken(currentLex);

	currentLex = eStatement(currentLex, s, fsys);

	return currentLex;
}

LexNode *eReadStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	Symbol *temp;
	Set emptySet;

	emptySet.len = 0;

	currentLex = getToken(currentLex);

	if(currentLex->symbol.tokenType == identsym) {
		
		if((temp = getSymbol(s, currentLex->symbol.name, s->lexLevel, 0)) == NULL)
			error(s, 11); // undeclared identifier	

		else {		
			if(temp->kind != VAR)
				error(s, 12); // assignment to constant or proc not allowed
		}

		currentLex = getToken(currentLex);	
	}
	
	else
		currentLex = test(currentLex, s, fsys, &emptySet, 26);
	
	return currentLex;
}

LexNode *eWriteStatement(LexNode *currentLex, SymbolTable *s, Expression *e, Set *fsys) {

	currentLex = eReadStatement(currentLex, s, e, fsys);

	return currentLex;
}

LexNode *eStatement(LexNode *currentLex, SymbolTable *s, Set *fsys) {

	Expression *e = createNewExpression();	
	Set emptySet;

	emptySet.len = 0;

	currentLex = test(currentLex, s, &(s->m.begStatement), fsys, 7);
	
	if(currentLex->symbol.tokenType == identsym)
		currentLex = eIdentStatement(currentLex, s, e, fsys);

	else if(currentLex->symbol.tokenType == callsym) 
		currentLex = eCallStatement(currentLex, s, e, fsys);

	else if(currentLex->symbol.tokenType == beginsym)
		currentLex = eBeginStatement(currentLex, s, e, fsys);

	else if(currentLex->symbol.tokenType == ifsym) 
		currentLex = eIfStatement(currentLex, s, e, fsys);

	else if(currentLex->symbol.tokenType == whilesym) 
		currentLex = eWhileStatement(currentLex, s, e, fsys);

	else if(currentLex->symbol.tokenType == readsym) 
		currentLex = eReadStatement(currentLex, s, e, fsys);
	
	else if(currentLex->symbol.tokenType == writesym) 
		currentLex = eWriteStatement(currentLex, s, e, fsys);

	destroyExpression(e);

	currentLex = test(currentLex, s, fsys, &emptySet, 19);

	return currentLex;
}

LexNode *eConstDeclaration(LexNode *currentLex, SymbolTable *s, Set *fsys) {
	
	Symbol *temp;
	Set constBeg, amendedFsys;

	constBeg.len = 0;
	constBeg.includedTokens[constBeg.len++] = identsym;

	amendedFsys = *fsys;
	amendedFsys.includedTokens[amendedFsys.len++] = eqlsym;

	
	do {	
		currentLex = getToken(currentLex);
	
		temp = &(currentLex->symbol); // store symbol location
		
		if(temp->tokenType != identsym)
			currentLex = test(currentLex, s, &constBeg, fsys, 4);

		currentLex = getToken(currentLex);

		if(currentLex->symbol.tokenType == becomessym)
			error(s, 3); 

		if(currentLex->symbol.tokenType == eqlsym || currentLex->symbol.tokenType == becomessym) {
		
			currentLex = getToken(currentLex);

			if(currentLex->symbol.tokenType != numbersym)
				error(s, 2); // = must be followed by a number
			else {
				temp->kind = CONST;
				temp->level = s->lexLevel;
				temp->val = atoi(currentLex->symbol.name);
				temp->proc = s->proc[s->lexLevel];

				addSymbol(s, temp);  // add to symbol table

			}	
			
			currentLex = getToken(currentLex);
		}

		else
			error(s, 3);

	} while(currentLex->symbol.tokenType == commasym || currentLex->symbol.tokenType == identsym);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5); // semicolon or comma missing
	else
		currentLex = getToken(currentLex);

	return currentLex;
}

LexNode *eVarDeclaration(LexNode *currentLex, SymbolTable *s, Set *fsys) {

	int semiFound = 0;
	Set varBeg;

	varBeg.len = 0;
	varBeg.includedTokens[varBeg.len++] = identsym;

	do {
		if(currentLex->symbol.tokenType != identsym) {

			currentLex = getToken(currentLex);
			currentLex = test(currentLex, s, &varBeg, fsys, 4);
		}

		if(currentLex->symbol.tokenType == identsym) {
			
			currentLex->symbol.kind = VAR;
			currentLex->symbol.level = s->lexLevel;
			currentLex->symbol.addr = s->count + 4; // gets next slot in memory
			currentLex->symbol.proc = s->proc[s->lexLevel];

			addSymbol(s, &(currentLex->symbol)); // add to symbol table
			currentLex = getToken(currentLex);

			if(currentLex->symbol.tokenType == identsym)
				error(s, 5);
		}

	} while(currentLex->symbol.tokenType == commasym || currentLex->symbol.tokenType == identsym);
	
	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5); // semicolon or comma missing
	else		
		currentLex = getToken(currentLex);
	
	return currentLex;
}

LexNode *eProcDeclaration(LexNode *currentLex, SymbolTable *s, Set *fsys) {
	
	Symbol *temp;
	Set procBeg, amendedFsys, tempSet;
	
	procBeg.len = 0;
	procBeg.includedTokens[procBeg.len++] = identsym;

	amendedFsys = *fsys;

	if(!contains(amendedFsys.includedTokens, amendedFsys.len, semicolonsym))
		amendedFsys.includedTokens[amendedFsys.len++] = semicolonsym;

	currentLex = getToken(currentLex);
	temp = &(currentLex->symbol);

	if(temp->tokenType == identsym) {

		(s->lexLevel)++;	
	
		temp->kind = PROC;
		temp->level = s->lexLevel;         // L
		temp->addr = s->codeLength;        // M 
		s->proc[s->lexLevel] = temp->name; // track procedure name at each level

		addSymbol(s, temp);	

		currentLex = getToken(currentLex);
	}

	else
		error(s, 4);	

	if(currentLex->symbol.tokenType == semicolonsym)
		currentLex = getToken(currentLex);
	else
		error(s, 6); // incorrect symbol after proc declaration
		
	currentLex = eBlock(currentLex, s, &amendedFsys);

	if(currentLex->symbol.tokenType != semicolonsym)
		error(s, 5);
	else {
		tempSet = s->m.begStatement;
		tempSet.includedTokens[tempSet.len++] = procsym;

		currentLex = getToken(currentLex);
		currentLex = test(currentLex, s, &tempSet, fsys, 6); 
	}
	
	(s->lexLevel)--; // revert state

	return currentLex;
}

LexNode *eBlock(LexNode *currentLex, SymbolTable *s, Set *fsys) {

	int repeat = 0;
	Set amendedFsys, emptySet;

	amendedFsys = *fsys;
	emptySet.len = 0;
		
	if(!contains(amendedFsys.includedTokens, amendedFsys.len, endsym))
		amendedFsys.includedTokens[amendedFsys.len++] = endsym;
 
	if(s->lexLevel > MAX_LEXI_LEVELS)
		error(s, 32); 
	
	while(currentLex->symbol.tokenType == constsym 
	      || currentLex->symbol.tokenType == varsym) {
	
		if(repeat)
			error(s, 27);

		if(currentLex->symbol.tokenType == constsym) 
			currentLex = eConstDeclaration(currentLex, s, fsys);
			
		if(currentLex->symbol.tokenType == varsym) 
			currentLex = eVarDeclaration(currentLex, s, fsys);
	
		repeat++;

		if(repeat > 1)
			break;
	}

	while(currentLex->symbol.tokenType == procsym)
		currentLex = eProcDeclaration(currentLex, s, fsys);

	currentLex = eStatement(currentLex, s, &amendedFsys);

	currentLex = test(currentLex, s, fsys, &emptySet, 8);
	
	return currentLex;
}

void initializeSets(SymbolTable *s) {

	s->m.begBlock.len = s->m.begStatement.len = s->m.begCondition.len
	= s->m.begExpression.len = s->m.begFactor.len = 0;

	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = constsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = varsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = procsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = identsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = callsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = beginsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = ifsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = whilesym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = readsym;
	s->m.begBlock.includedTokens[(s->m.begBlock.len)++] = writesym;

	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = identsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = callsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = beginsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = ifsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = whilesym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = readsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = writesym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = semicolonsym;
	s->m.begStatement.includedTokens[(s->m.begStatement.len)++] = endsym;

	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = oddsym;
	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = plussym;
	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = minussym;
	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = lparentsym;
	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = identsym;
	s->m.begCondition.includedTokens[(s->m.begCondition.len)++] = numbersym;

	s->m.begExpression.includedTokens[(s->m.begExpression.len)++] = plussym;
	s->m.begExpression.includedTokens[(s->m.begExpression.len)++] = lparentsym;
	s->m.begExpression.includedTokens[(s->m.begExpression.len)++] = minussym;
	s->m.begExpression.includedTokens[(s->m.begExpression.len)++] = identsym;
	s->m.begExpression.includedTokens[(s->m.begExpression.len)++] = numbersym;

	s->m.begFactor.includedTokens[(s->m.begFactor.len)++] = identsym;
	s->m.begFactor.includedTokens[(s->m.begFactor.len)++] = numbersym;
	s->m.begFactor.includedTokens[(s->m.begFactor.len)++] = lparentsym;
}

int errorRecovery(LexNode *currentLex) {

	int numErrors;
	Set tempSet;
	LexNode *head = currentLex;

	SymbolTable *s = calloc(1, sizeof(SymbolTable));	
	initializeSets(s);

	s->proc[0] = "";
	
	tempSet.len = 0;
	tempSet.includedTokens[tempSet.len++] = periodsym;
	tempSet.includedTokens[tempSet.len++] = semicolonsym;
            
	currentLex = eBlock(currentLex, s, &tempSet);

	if(currentLex->symbol.tokenType != periodsym)
		error(s, 9);  // period expected	

	numErrors = s->numErrors;
	
	free(s);

	return numErrors;
}
