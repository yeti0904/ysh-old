#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
using std::string;
using std::vector;
using std::map;

int interpret(string in, map <string, string> &variables) {
	// variables
	string          programpath;
	string          workingpath;
	pid_t           childpid;
	int             status;
	bool            execute = true;
	vector <string> args;
	string          reading;
	bool            inString = false;

	// seperate string into arguments
	for (size_t i = 0; i<=in.length(); ++i) {
		if (((in[i] == ' ') || (i == in.length())) && !inString) {
			// handle variable
			if ((reading.length() >= 4) && (reading.substr(0, 2) == "${") && (reading[reading.length()-1] == '}')) {
				reading = variables[reading.substr(2, reading.length()-3)];
			}
			/*else if (reading[0] == '$') {
				reading = getenv(reading.substr(1).c_str());
			}*/
			args.push_back(reading);
			reading = "";
		}
		else if ((in[i] == '"') || (in[i] == '\'')) {
			inString = !inString;
		}
		else
			reading += in[i];
	}

	// turn into a C string array
	const char **aargv = new const char* [args.size()+2];
	for (int j = 0; j<args.size(); ++j) {
		aargv[j]      = args[j].c_str();
	}
	aargv[args.size()] = NULL;
	
	// get program path
	programpath = args[0];

	// shell command / code
	execute = true;
	if (args[0] == "exit") {
		exit(0);
	}
	if (args[0] == "help") {
		execute = false;
		printf("ysh help\n========\n"
		"cd             - change directory to home\n"
		"cd [path]      - change directory to path\n"
		"exit           - exit ysh\n\n"
		"useful commands\n===============\n"
		"ls [path]         - view contents of a path\n"
		"cat [filename]    - view contents of a file\n"
		"set [key] [value] - set a variable\n\n"
		"substitution\n============\n"
		"$NAME             - enviroment variable\n"
		"${name}           - ysh variable\n"
		);
	}
	if (args[0] == "cd") {
		execute = false;
		if ((args.size() == 1) || (args[1] == "")) {
		chdir(getenv("HOME"));
		}
		else {
			chdir(args[1].c_str());
		}
	}
	if (args[0] == "set") {
		execute = false;
		if (args.size() == 1) {
			printf("Usage: set [key] [value]\n");
		}
		else {
			variables[args[1]] = args[2];
		}
	}
	if (args[0][0] == '#') {
		execute = false;
	}

	// execute
	if (execute) {
		childpid = fork();
		if (childpid == 0) {
			if (execvp(programpath.c_str(), (char**) aargv) == -1) {
				if (args[0] != "") perror(in.c_str());
			}
			exit(0);
		}
		if (childpid > 0) {
			childpid = wait(&status);
		}
	}
	return 0;
}