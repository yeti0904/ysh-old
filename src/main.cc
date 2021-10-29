#include <cstdio>
#include <cstdlib>
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
		o_fhnd << "alias ls \"ls --color=auto\"\n";
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
		prompt = string("\x1b[32m") + string(getenv("USER")) + string(":\x1b[36m") + string(get_current_dir_name()) + "\x1b[0m> ";
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