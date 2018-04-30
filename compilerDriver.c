#include "definitions.h"
#include "vm.c"
#include "lex.c"
#include "parser.c"
#include "errorRecovery.c"

void validateInput(int argc) {

	if(argc < 2) {

		printf("Missing arguments.  [Program] [input file] {flags (-l, -a, -v)}\n");
		exit(500);
	}
}

void getFlags(int argc, char **argv, int *lflag, int *aflag, int *vflag) {

	int i;
	char c;

	for(i = 2; i < argc; i++) {

		if(strlen(argv[i]) != 2 || argv[i][0] != '-') {

			printf("Invalid flag.  Use format -l, -a, -v)\n");
			exit(501);
		}

		c = argv[i][1];

		if(c == 'l')
			*lflag = 1;
		else if(c == 'a')
			*aflag = 1;
		else if(c == 'v')
			*vflag = 1;
		else {
			printf("Character '%c' is not a valid flag (-l, -a, -v)\n", c);
			exit(502);
		}
	}
}

int main(int argc, char **argv) {
	
	int i, lflag, aflag, vflag, numErrors;
	char *filename;
	LexNode *headLex;
	
	lflag = aflag = vflag = 0;

	validateInput(argc);
	getFlags(argc, argv, &lflag, &aflag, &vflag);
	filename = argv[1];

	headLex = runLexicalAnalyzer(filename); // get lex units

	printf("\nSource File:\n\n");
	printTextFile(filename);

	numErrors = errorRecovery(headLex);

	if(numErrors == 0) { 

		printf("### No errors, program is syntactically correct. ###\n\n");	

		runParser(headLex); // create assembly file

		if(lflag) {
			
			printLex(headLex, 1);  // print lex table
			printLex(headLex, 2);  // print lex list
			printLex(headLex, 3);  // print symbolic lex list
		}
		
		if(aflag) {

			printf("Generated Assembly:\n\n");	
			printTextFile(ASM_FINAL); // print final assembly		
		}

		virtualMachine(ASM_FINAL, vflag); 			

		printf("Output File:\n\n");
		printTextFile(VM_OUT);
	}
	
	else
		printf("### There are at least %d errors in the program ###\n", numErrors);

	destroyLexTable(headLex);	

	return 0;
}
