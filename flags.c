#include "flags.h"

int handleFlags(int argc, char** argv){
	int argIndex = 1;
	while (argIndex < argc){
		if (!strcmp(argv[argIndex], "-v")){
			turnOnVerbose();
			logServer("Verbose mode active\n");
		}
		argIndex++;
	}
	return 0;
}

