#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <ncurses.h>

// Macros you might still need for other parts of your code.
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_DELIM " \t\r\n\a"


int lsh_cd(const std::vector<std::string>& args);
int lsh_help(const std::vector<std::string>& args);
int lsh_exit(const std::vector<std::string>& args);
int lsh_http_server(const std::vector<std::string>& args);
int lsh_num_builtins();

// Global vector of built-in command names.
std::vector<std::string> builtin_str = { "cd", "help", "exit", "http-server", "blue" };

// Define a function pointer type for built-in commands.
using builtin_func_t = int(*)(const std::vector<std::string>&);

// Global vector of built-in function pointers (order must match builtin_str).
std::vector<builtin_func_t> builtin_func = { lsh_cd, lsh_help, lsh_exit, lsh_http_server };

std::string lsh_read_line() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::vector<std::string> lsh_split_line(const std::string &line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {  // tokenizes by whitespace by default
        tokens.push_back(token);
    }
    return tokens;
}

// Builtin commands //
int lsh_cd(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "lsh: expected argument to \"cd\"\n";
    } else {
        // chdir expects a C-style string.
        if (chdir(args[1].c_str()) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(const std::vector<std::string>& args) {
    std::cout << "Lukes LSH\n";
    std::cout << "Type program names and arguments, and hit enter.\n";
    std::cout << "The following are built in:\n";
    for (size_t i = 0; i < builtin_str.size(); i++) {
        std::cout << " " << builtin_str[i] << "\n";
    }
    std::cout << "Use the man command for information on other programs.\n";
    return 1;
}

int lsh_exit(const std::vector<std::string>& args) {
    return 0;
}

// setup the directory for html files in the linux srv/www directory 
int lsh_http_server(const std::vector<std::string>& args) {
    std::vector<std::string> new_args;
    std::string file = "./http";
    new_args.push_back(file);
    return lsh_launch(new_args);
}

int lsh_num_builtins() {
    return builtin_str.size();
}

// Main functions //

// launches the child process using execvp which is passed any command line args 
int lsh_launch(const std::vector<std::string>& args)
{
    std::vector<char*> argv;
    for (const auto &arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
        // Child process: execute the command.
        if (execvp(argv[0], argv.data()) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking.
        perror("lsh");
    } else {
        // Parent process: wait for child to finish.
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int lsh_execute(const std::vector<std::string>& args) {
    if (args.empty() || args[0].empty()) {
        return 1;
    }

    // Check if the command matches a built-in command.
    for (int i = 0; i < lsh_num_builtins(); i++) {
        if (args[0] == builtin_str[i]) {
            return (*builtin_func[i])(args);
        }
    }

    // Otherwise, launch it as an external command.
    return lsh_launch(args);
}

void lsh_loop() {
    int status;
    do {
        std::cout << "> ";
        std::string line = lsh_read_line();
        std::vector<std::string> args = lsh_split_line(line);
        status = lsh_execute(args);
    } while (status);
}


int main(int argc, char **argv)
{
    lsh_loop();
    return EXIT_SUCCESS;
}




