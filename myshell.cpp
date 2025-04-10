#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <wait.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;

#define pathLen 4096

char theLastPath[pathLen] = {0};
char CurrentPath[pathLen];

struct Command
{
    vector<string> args;
    string input;
    string output;
    bool isApend = false;
};

void catch_child(int signo)
{
    pid_t pid;
    while (1)
    {
        pid = waitpid(-1, NULL, WNOHANG);
        if (pid <= 0)
            break;
    }
}

void set_sigchld()
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = catch_child;
    sigaction(SIGCHLD, &act, NULL);
}

// 系统调用错误
void sys_error(const char *systemcall)
{
    perror(systemcall);
    exit(-1);
}

// 读取用户输入
void read_in(string &strp)
{
    string str;
    /* if (!(getline(cin, strp)))
         exit(1);*/
    cerr << "\x1b[1;34m" << CurrentPath << "\x1b[0m";
    char *input = readline("\x1b[1;34m➜➜➜➜➜\x1b[0m");
    if (!input)
        exit(1);
    strp = input;
    free(input);
}

// 按空格分割成参数
vector<string> splite_argv(const string &strp)
{
    vector<string> args;
    istringstream stream(strp);
    string arg;
    while (stream >> arg)
    {
        args.push_back(arg);
    }

    return args;
}

// 分割参数|
vector<vector<string>> splite_pipe(const string &strp)
{
    vector<vector<string>> cmds;
    istringstream stream(strp);
    string cmd;
    while (getline(stream, cmd, '|'))
    {
        cmds.push_back(splite_argv(cmd));
    }
    return cmds;
}

vector<Command> splite_command(const string &strp)
{

    vector<Command> commands;
    istringstream stream(strp);
    string command;
    while (getline(stream, command, '|'))
    {
        istringstream single_stream(command);
        struct Command cmd;
        string arg;
        while (single_stream >> arg)
        {
            if (arg == ">")
            {
                single_stream >> cmd.output;
            }
            else if (arg == "<")
            {
                single_stream >> cmd.input;
            }
            else if (arg == ">>")
            {
                single_stream >> cmd.output;
                cmd.isApend = true;
            }
            else
            {

                cmd.args.push_back(arg);
            }
        }
        commands.push_back(cmd);
    }
    return commands;
}

// cd
void cd(vector<string> &args, char *theLastPath)
{
    if (args.size() < 2 || strcmp(args[1].c_str(), "~") == 0)
    {
        const char *homedir = getenv("HOME");
        if (chdir(homedir) != 0)
        {
            perror("cd");
            return;
        }
    }
    else if (args.size() < 2 || strcmp(args[1].c_str(), "-") == 0)
    {

        if (theLastPath[0] == '\0')
        {
            getcwd(theLastPath, pathLen);
            // cout << "________________" << endl;
        }
        cout << theLastPath << endl;
        if (chdir(theLastPath) != 0)
        {
            perror("cd");
            return;
        }
    }
    else
    {
        if (chdir(args[1].c_str()) != 0)
            perror("cd");
        return;
    }
}

vector<char *> transfer(vector<string> &cmd)
{
    vector<char *> args;
    for (auto &arg : cmd)
    {
        args.push_back(&arg[0]);
    }
    args.push_back(nullptr);
    return args;
}

void cmd_pipe(vector<Command> &commands, int &num, bool &houtai)
{

    vector<vector<int>> fd(num, vector<int>(2));
    // 建立管道
    for (int i = 0; i < num - 1; i++)
    {
        if (pipe(fd[i].data()) == -1)
            sys_error("pipe");
    }
    int i = 0;
    vector<pid_t> pid;
    // 创建子进程
    for (i = 0; i < num; i++)
    {
        pid.push_back(fork());
        if (pid[i] == 0)
            break;
    }

    if (pid[i] == 0)
    {
        if (i > 0)
        {
            dup2(fd[i - 1][0], STDIN_FILENO);
        }
        else if (!commands[0].input.empty())
        {
            int fD = open(commands[0].input.c_str(), O_RDONLY);
            dup2(fD, STDIN_FILENO);
            close(fD);
        }
        if (i < num - 1)
        {
            dup2(fd[i][1], STDOUT_FILENO);
        }
        else if (!commands[num - 1].output.empty())
        {
            int fD;
            if (commands[num - 1].isApend)
                fD = open(commands[num - 1].output.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fD = open(commands[num - 1].output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fD, STDOUT_FILENO);
            close(fD);
        }

        for (int j = 0; j < num - 1; j++)
        {
            close(fd[j][1]);
            close(fd[j][0]);
        }
        vector<char *> char_args = transfer(commands[i].args);
        char_args.push_back(nullptr);
        if (execvp(char_args[0], char_args.data()) == -1)
            sys_error("execvp");
        exit(0);
    }
    else if (pid[i] > 0) // 关闭父进程的所有通道i
    {
        for (int j = 0; j < num - 1; j++)
        {
            close(fd[j][1]);
            close(fd[j][0]);
        }
        if (houtai)
        {
            // cout << "-------------------------" << endl;
            return;
        }
        for (int j = 0; j < num; j++)
            waitpid(pid[j], NULL, 0);
    }
    else
        sys_error("fork");
}

int main(int argc, char *argv[])
{

    sigset_t set, old;
    set_sigchld();

    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    sigprocmask(SIG_BLOCK, &set, &old);

    getcwd(CurrentPath, pathLen);

    while (1)
    {
        bool exe = true;
        string str;
        bool houtai = false;
        // 调取命令行参数
        read_in(str);
        vector<Command> commands = splite_command(str);
        int num2 = commands.size();
        if (num2 == 0)
            continue;
        int num = commands[0].args.size();
        if (num == 0)
            continue;
        // cout << args[1] << endl;
        if (num != 0 && strcmp(commands[num2 - 1].args[commands[num2 - 1].args.size() - 1].c_str(), "&") == 0)
        {
            // cout << "-------------------" << endl;
            houtai = true;

            commands[num2 - 1].args.pop_back();
        }

        if (num != 0 && commands[0].args[0] == "cd")
        {
            cd(commands[0].args, theLastPath);
            strcpy(theLastPath, CurrentPath);
            getcwd(CurrentPath, pathLen);
            // cout << "currentPaTH:" << CurrentPath << endl;
            // cout << "theLastPath:" << theLastPath << endl;
            continue;
        }
        else if (num != 0 && commands[0].args[0].rfind("./", 0) == 0)
        {
            exe = false;
            int pid = fork();
            if (pid == 0)
            {
                vector<char *> char_args = transfer(commands[0].args);
                char_args.push_back(nullptr);
                if (execv(char_args[0], char_args.data()) == -1)
                    sys_error("execv");
            }
            else if (pid < 0)
                sys_error("fork");
            else
                waitpid(pid, NULL, 0);
        }
        else if (num != 0 && commands[0].args[0] == "exit")
        {
            // printf("recieve exit\n");
            exit(0);
        }
        if (exe)
            cmd_pipe(commands, num2, houtai);
    }
    return 0;
}