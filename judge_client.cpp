#define BUFF_SIZE 512
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <mysql/mysql.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <wait.h>
using namespace std;

// extern int errno;

MYSQL *conn;
static char host_name[BUFF_SIZE];
static char user_name[BUFF_SIZE];
static char password[BUFF_SIZE];
static char db_name[BUFF_SIZE];
static int port_number;

/**
 * 通过C API调用mysql
 */
void init_mysql()
{
    conn = mysql_init(NULL);
    const char timeout = 30;
    //配置连接时间
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    //mysql_real_connect(conn, "rm-bp14419zgc8077s9hjo.mysql.rds.aliyuncs.com", "mo", "WjC120211", "online_judge", 3306, NULL, 0)
    if (mysql_real_connect(conn, host_name, user_name, password, db_name, port_number, NULL, 0))
    {
    }

    string sql = "select username from users;";

    mysql_query(&mysql, sql.c_str());

    MYSQL_RES *res = mysql_store_result(&mysql);

    int num_fields = mysql_num_fields(res);

    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    while (MYSQL_ROW row = mysql_fetch_row(res)) //获取整条数据内容
    {
        for (int i = 0; i < num_fields; i++)
        {
            if (NULL == row[i])
            {
                cout << " NULL";
            }
            else
            {
                cout << " " << row[i];
            }
        }
        cout << endl;
    }
    mysql_close(&mysql);
}
/**
 * 执行cmd命令
 */
int execute_cmd(const char *fmt, ...)
{
    char cmd[BUFFER_SIZE];
    // fill(cmd, cmd + BUFFER_SIZE, '-');
    int ret = 0;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    ret = system("echo 666");
    int i = 0;
    while (cmd[i] != '\0' && i < BUFFER_SIZE)
    {
        cout << cmd[i];
        i++;
    }
    printf("\n");
    va_end(ap);
    return ret;
}
/**
 * 编译
 */
int compile()
{
    int pid;

    // freopen("log/error.out", "w", stdout);
    char *arg[] = {"g++", "test.cc", "-o", "test", NULL};
    // char *arg[] = {"./alarm", NULL};
    pid = fork();
    if (pid < 0)
    {
        cout << "错误" << endl;
    }
    else if (pid == 0) //子进程
    {
        cout << "子函数执行！" << endl;
        freopen("log/ce.txt", "w", stderr);
        if (execvp("g++", arg) == -1)
        {
            printf("编译错误");
            // printf("错误代码=%d\n", errno);
            // char *mesg = strerror(errno);
            // printf("错误原因:%s\n", mesg);
        }
    }
    else //父进程
    {
        int status = 0;
        waitpid(pid, &status, 0);
        status = get_file_size("log/ce.txt");
        printf("size:%d\n", status);
        // if (WIFEXITED(status) != 0)
        // {
        //     cout << "正常结束：返回值为：" << WEXITSTATUS(status) << endl;
        // }
        // else
        // {
        //     cout << "非正常结束";
        //     printf_wrongMessage();
        // }
        return status;
    }
}
void printf_wrongMessage(int status)
{
    printf("错误代码=%d\n", status);
    char *mesg = strerror(status);
    printf("错误原因:%s\n", mesg);
}
void printf_wrongMessage()
{
    printf("错误代码=%d\n", errno);
    char *mesg = strerror(errno);
    printf("错误原因:%s\n", mesg);
}
/**
 * 执行编译结果
 */
void run_solution()
{
    freopen("user.out", "w", stdout);
    freopen("data.in", "r", stdin);
    freopen("error.out", "a+", stderr);
    //ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl("./test", "./test", (char *)NULL);
}
/**
 * 查看运行结
 */
void watch_solution(pid_t pidApp)
{
    int status;
    struct rusage ruse;
    wait4(pidApp, &status, 0, &ruse);
    //get_proc_status(pidApp, "VmPeak:");
    if (WIFEXITED(status))
    {
        cout << "子进程正常运行结束" << endl;
    }
    else
    {
        if (WIFSIGNALED(status))
        {
            printf_wrongMessage(WTERMSIG(status));
        }
    }
    //ptrace(PTRACE_KILL, pidApp, NULL, NULL); //杀死子进程，停止执行
}
void judge_solution()
{
    int res = compare("data.in", "user.out");
    if (res)
    {
        cout << "答案正确" << endl;
    }
    else
    {
        cout << "答案错误" << endl;
    }
}
int compare(const char *file1, const char *file2)
{
    FILE *fp1, *fp2;
    char *s1, *s2, *p1, *p2;
    s1 = new char[4096], s2 = new char[4096];
    fp1 = fopen(file1, "r");
    cout << "打开文件f1" << endl;
    for (p1 = s1; EOF != fscanf(fp1, "%s", p1);)
    {
        while (*p1)
        {
            cout << *p1;
            p1++;
        }
    }
    fclose(fp1);
    cout << "打开文件f1" << endl;
    cout << "打开文件f2" << endl;
    fp2 = fopen(file2, "r");
    for (p2 = s2; EOF != fscanf(fp2, "%s", p2);)
    {
        while (*p2)
        {
            cout << *p2;
            p2++;
        }
    }
    fclose(fp2);
    cout << "打开文件f2" << endl;
    if (strcmp(s1, s2) == 0)
    {
        delete[] s1;
        delete[] s2;
        return 1;
    }
    else
    {
        delete[] s1;
        delete[] s2;
        return 0;
    }
}
int get_proc_status(pid_t pid, const char *mark)
{
    FILE *file;
    char fileName[BUFFER_SIZE], buf[BUFFER_SIZE];

    sprintf(fileName, "/proc/%d/status", pid);
    cout << "fileName" << fileName << endl;
    file = fopen(fileName, "r");
    int len = strlen(mark), res;
    cout << "文件打开成功" << file << endl;
    while (file && fgets(buf, BUFFER_SIZE - 1, file))
    {
        if (strncmp(mark, buf, len) == 0)
        {
            printf("%s", buf);
            sscanf(buf + len, "%d", &res);
            cout << (res << 10) << endl;
        }
    }

    if (file)
        fclose(file);
    return 0;
}
/**
 * 获取制定文件的大小 
 */
long get_file_size(const char *filename)
{
    struct stat f_stat;

    if (stat(filename, &f_stat) == -1)
    {
        return 0;
    }
    return (long)f_stat.st_size;
}

void get_solution_info_MySQL(int solution_id)
{
    char sql[BUFFER_SIZE];
    // get the problem id and user id from Table:solution
    sprintf(sql, "select problem_id, user_id, language FROM solution where solution_id=%d", solution_id);
    mysql_real_query(conn, sql, strlen(sql));
}

int main(int argc, char **argv)
{
    // testMysql();

    int i;
    for (i = 0; i < argc; i++)
    {
        printf("Argument %d is %s \n", i, argv[i]);
    }
    // system("cd ../ && mkdir 6666");
    // execute_cmd("test %s %d", "content", 16);
    int comile_flag = compile();
    if (comile_flag != 0)
    {
        //编译错误，更新数据库并退出
    }
    else
    {
        //更新数据库
    }
    /*
    1.读取输入输出文件（下载或者访问）
    2.设置时间限制，内存限制
    3. (1)子进程运行程序
       (2)父进程等待结束后判断程序输出和正确结果的异同
    4.更新结果
    */
    /*
    pid_t pidApp = fork();
    if (pidApp == 0) //子进程
    {
        run_solution();
    }
    else
    {
        cout << "pid = " << pidApp << endl;
        sleep(2);
        watch_solution(pidApp);
        judge_solution();
    }
    */

    return 0;
}