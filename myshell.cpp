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
        vector<vector<string>> args = splite_pipe(str);

        // cout << args[1] << endl;

        if (args[0][0] == "cd")
        {
            cout << "m" << endl;
            cd(args[0], theLastPath);
            strcpy(theLastPath, CurrentPath);
            getcwd(CurrentPath, pathLen);

            continue;
        }
    }
    return 0;
}