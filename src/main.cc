#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "scripting.hh"
using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;

bool replace(string& str, const string& from, const string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos) {
		return false;
	}
	str.replace(start_pos, from.length(), to);
	return true;
}

void signal_handler(int hi) {
	int signal = rl_pending_signal();
	putchar(10);
	string prompt = string("\x1b[32m") + string(getenv("USER")) + string(":\x1b[36m") + string(get_current_dir_name()) + "\x1b[0m> ";
	replace(prompt, getenv("HOME"), "~");
	printf("%s", prompt.c_str());
}

int main(int argc, char** argv) {
	if (argc >= 2 && (strcmp(argv[1], "--version") == 0)) {
		printf("ysh 1.9");
		return 0;
	}
	setenv("SHELL", argv[0], true);
	signal(SIGINT, signal_handler);
	rl_set_signals();

	bool inIf = false;

	map <string, string> variables;
	map <string, string> aliases;

	string conf_path = getenv("HOME");
	conf_path += "/.yshrc";
	if (access(conf_path.c_str(), F_OK) != 0) {
		ofstream o_fhnd;
		o_fhnd.open(conf_path);
		string defaultprompt;
		defaultprompt = "\x1b[32m&u:\x1b[36m&wd\x1b[0m> ";
		o_fhnd << "# ysh dotfile\n";
		o_fhnd << "# you can use this file for customising ysh\n";
		o_fhnd << "set YSH_PROMPT \"" + defaultprompt + "\"\n\n";
		o_fhnd << "# aliases\n";
		o_fhnd << "# create an alias with: alias <alias name> <command>\n";
		o_fhnd.close();
	}

	string line;
	ifstream fhnd;
	fhnd.open(conf_path);
	if (fhnd.is_open()) {
		inIf = false;
		string ifbool;
		while (getline(fhnd, line)) {
			if (line.substr(0, 3) == "if ") {
				inIf = true;
				ifbool = line.substr(3);
			}
			else if (line == "end") {
				inIf = false;
			}
			else if (!inIf || ((line.length() >= 5) && inIf && (variables[ifbool] == "true")))
				interpret(line, variables, aliases);
		}
		fhnd.close();
	}

	bool   script = false;
	string script_fname;
	for (size_t i = 1; i<argc; ++i) {
		script = true;
		script_fname = argv[i];
	}
	// variables
	char*           inr;
	string          in;
	bool            runshell;
	vector <string> args;
	string          reading;
	string          prompt;
	string          programpath;
	string          workingpath;
	pid_t           childpid;
	int             status;
	bool            execute;

	// use prompt if not running from a file
	if (script) runshell = false;
	else        runshell = true;

	while (runshell) {
		// take input
		char* currentdir = get_current_dir_name();
		prompt = variables["YSH_PROMPT"];
		replace(prompt, "&u", getenv("USER"));
		replace(prompt, "&wd", currentdir);
		free(currentdir);
		replace(prompt, getenv("HOME"), "~");
		inr    = readline(prompt.c_str());
		if (inr == NULL) {
			printf("readline error\n");
		}
		in     = inr;
		add_history(inr);
		free(inr);
		replace(in, "~", getenv("HOME"));
		interpret(in, variables, aliases);
	}

	if (script) {
		if (access(script_fname.c_str(), F_OK) == 0) {
			string line;
			ifstream fhnd;
			inIf = false;
			fhnd.open(script_fname);
			string ifbool;
			while (getline(fhnd, line)) {
				if (line.substr(0, 3) == "if ") {
					inIf = true;
					ifbool = line.substr(3);
				}
				else if (line == "end") {
					inIf = false;
				}
				else if (!inIf || ((line.length() >= 5) && inIf && (variables[ifbool] == "true")))
					interpret(line, variables, aliases);
			}
			fhnd.close();
		}
		else {
			printf("%s: no such file\n", script_fname.c_str());
		}
	}

	return 0;
}