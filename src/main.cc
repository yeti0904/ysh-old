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

	map <string, string> variables;

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
		interpret(in, variables);
	}

	if (script) {
		if (access(script_fname.c_str(), F_OK) == 0) {
			string line;
			ifstream fhnd;
			fhnd.open(script_fname);
			while (getline(fhnd, line)) {
				interpret(line, variables);
			}
			fhnd.close();
		}
		else {
			printf("%s: no such file\n", script_fname.c_str());
		}
	}

	return 0;
}