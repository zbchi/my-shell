#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <wait.h>
#include <cstring>
#include <fcntl.h>
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
    getline(cin, strp);
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
    else if (strcmp(args[1].c_str(), "-") == 0)
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

void cmd_pipe(vector<Command> &commands)
{

    int num = commands.size();
    vector<vector<int>> fd(num, vector<int>(2));
    // 建立管道
    for (int i = 0; i < num - 1; i++)
    {
        if (pipe(fd[i].data()) == -1)
            sys_error("pipe");
    }
    int i = 0;
    pid_t pid;
    // 创建子进程
    for (i = 0; i < num; i++)
    {
        pid = fork();
        if (pid == 0)
            break;
    }

    if (pid == 0)
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
        execvp(char_args[0], char_args.data());
        exit(0);
    }
    else // 关闭父进程的所有通道i
    {
        for (int j = 0; j < num - 1; j++)
        {
            close(fd[j][1]);
            close(fd[j][0]);
        }
        for (int j = 0; j < num; j++)
            wait(nullptr);
    }
}

int main(int argc, char *argv[])
{
    getcwd(CurrentPath, pathLen);

    while (1)
    {
        cerr << "myshell ➜";
        string str;

        // 调取命令行参数
        read_in(str);
        vector<Command> commands = splite_command(str);

        // cout << args[1] << endl;
        if (commands[0].args[0] == "cd")
        {
            cd(commands[0].args, theLastPath);
            strcpy(theLastPath, CurrentPath);
            getcwd(CurrentPath, pathLen);
            // cout << "currentPaTH:" << CurrentPath << endl;
            // cout << "theLastPath:" << theLastPath << endl;
            continue;
        }
        else if (commands[0].args[0] == "exit")
        {
            exit(0);
        }
        cmd_pipe(commands);
    }
    return 0;
}