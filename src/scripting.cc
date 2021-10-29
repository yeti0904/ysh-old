#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
using std::string;
using std::vector;
using std::map;

int interpret(string in, map <string, string> &variables, map <string, string> &aliases) {
	// variables
	string          programpath;
	string          workingpath;
	pid_t           childpid;
	int             status;
	bool            execute = true;
	vector <string> args;
	string          reading;
	bool            inString = false;

	string cmd = in.substr(0, in.find(' '));
	if (cmd == "") {
		cmd = in;
	}
	if (aliases[cmd] != "") {
		in.replace(0, in.find(" ") - 1, aliases[cmd]);
	}

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
			if (reading != "") args.push_back(reading);
			reading = "";
		}
		else if ((in[i] == '"') || (in[i] == '\'')) {
			inString = !inString;
		}
		else
			if (inString || (in[i] != 9)) reading += in[i];
	}

	// turn into a C string array
	const char **aargv = new const char* [args.size()+2];
	for (int j = 0; j<args.size(); ++j) {
		aargv[j]      = args[j].c_str();
	}
	aargv[args.size()] = NULL;
	
	// get program path
	if (args.size() != 0) programpath = args[0];

	// shell command / code
	execute = true;
	if (args.size() != 0) {
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
		if (args[0] == "ifand") {
			if (args.size() >= 4) {
				bool result = true;
				for (size_t i = 2; i<args.size(); ++i) {
					if (args[i] != "true") {
						result = false;
					}
				}
				variables[args[1]] = result? "true" : "false";
			}
			else {
				printf("Usage: ifand [result] [var1] [var2] ...\n");
			}
		}
		if (args[0] == "strcmp") {
			execute = false;
			if (args.size() == 4) {
				variables[args[1]] = args[2] == args[3]? "true": "false";
			}
			else {
				printf("Usage: strcmp [result] [str1] [str2]");
			}
		}
		if (args[0] == "ifnot") {
			execute = false;
			if (args.size() == 3) {
				variables[args[1]] = variables[args[2]] != "true"? "true" : "false";
			}
			else {
				printf("Usage: ifnot [result] [boolean]\n");
			}
		}
		if (args[0] == "input") {
			execute = false;
			if (args.size() == 3) {
				variables[args[1]] = string(readline(args[2].c_str()));
			}
			else {
				printf("Usage: input [result] [prompt]");
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
		if (args[0] == "alias") {
			execute = false;
			if (args.size() == 1) {
				printf("Usage: alias [key] [value]\n");
			}
			else {
				aliases[args[1]] = args[2];
			}
		}
		if (args[0][0] == '#') {
			execute = false;
		}
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