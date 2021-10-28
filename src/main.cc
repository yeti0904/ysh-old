#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
using std::string;
using std::vector;

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
	chdir(getenv("HOME"));
	setenv("SHELL", argv[0], true);
	signal(SIGINT, signal_handler);
	rl_set_signals();
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

		// seperate string into arguments
		args   = {};
		reading = "";
		for (size_t i = 0; i<=in.length(); ++i) {
			if ((in[i] == ' ') || (i == in.length())) {
				// handle variable
				if (reading[0] == '$') {
					reading = getenv(reading.substr(1).c_str());
				}
				args.push_back(reading);
				reading = "";
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
			return 0;
		}
		if (args[0] == "help") {
			execute = false;
			printf("ysh help\n========\n"
			"cd             - change directory to home\n"
			"cd [path]      - change directory to path\n"
			"exit           - exit ysh\n\n"
			"useful commands\n===============\n"
			"ls [path]      - view contents of a path\n"
			"cat [filename] - view contents of a file\n"
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
	}
	printf("hello\n");
	return 0;
}