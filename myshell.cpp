#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <wait.h>
using namespace std;

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

// 分割成参数
vector<char *> splite_argv(const string &strp)
{
    vector<string> tmpstr;
    istringstream stream(strp);
    string arg;
    while (stream >> arg)
    {
        tmpstr.push_back(arg);
    }

    vector<char *> args;
    for (auto &argg : tmpstr)
    {
        args.push_back(&argg[0]);
    }

    return args;
}

int main(int argc, char *argv[])
{
    cerr << "myshell->";
    string str;

    // 调取命令行参数
    read_in(str);
    vector<char *> args = splite_argv(str);

    pid_t pid = fork();
    if (pid < 0)
        sys_error("fork");

    // 给vector<char*>末尾加上NULL
    args.push_back(nullptr);
    if (pid == 0)
    {
        if ((execvp(args[0], args.data())) == -1)
            sys_error("execvp");
    }
    if (pid > 0)
    {
        wait(NULL);
    }
    return 0;
}