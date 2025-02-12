#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <wait.h>
#include <cstring>
using namespace std;

#define pathLen 4096

char theLastPath[pathLen] = {0};
char CurrentPath[pathLen];

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

// 分割参数|
vector<vector<string>> splite_pipe(const string &strp){
    vector<vector<args>>}

// 分割成参数
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
            cout << theLastPath << endl;
            // cout << "________________" << endl;
        }
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

int main(int argc, char *argv[])
{

    while (1)
    {
        cerr << "myshell ➜";
        string str;

        // 调取命令行参数
        read_in(str);
        vector<string> args = splite_argv(str);

        // cout << args[1] << endl;

        if (args[0] == "cd")
        {
            cd(args, theLastPath);
            strcpy(theLastPath, CurrentPath);
            getcwd(CurrentPath, pathLen);

            continue;
        }

        pid_t pid = fork();
        if (pid < 0)
            sys_error("fork");

        if (pid == 0)
        {
            vector<char *> char_args;
            for (auto &arg : args)
            {
                char_args.push_back(&arg[0]);
            }
            // 给vector<char*>末尾加上NULL
            char_args.push_back(nullptr);
            if ((execvp(char_args[0], char_args.data())) == -1)
                sys_error("execvp");
        }
        if (pid > 0)
        {
            wait(nullptr);
        }
    }
    return 0;
}