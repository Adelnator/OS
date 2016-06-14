#include <unistd.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <signal.h>
#include <wait.h>
#include <iostream>

using namespace std;

struct call_my {
    string name;
    vector<string> args;
    call_my(string s, vector<string> mas) {
        name = s;
        args = mas;
    }
    call_my(string s) {
        name = s;
    }
    call_my() {
    }
};

vector <call_my> programs;

int have_read_all[10];

int num_in = 0;

void read_until(int fd, char* buf, size_t count)
{
    for (int i = 0; i < 10; i++) {
        have_read_all[i] = 0;
    }
    num_in = 0;

    int have = 0;
    int res = 0;
    num_in = 0;

    while (1)
    {
        have = (int) read(fd, buf + res, count);
        if (buf[have] == '\0') {
            break;
        }
        if (have == 0) {
            break;
        } else if (have == -1) {
            have_read_all[num_in] = -1;
            break;
        } else if (have == count) {
            have_read_all[num_in] = have + res;
            break;
        }
        res += have;
        count -= have;
    }

    for (int i = 0; i < have; i++) {
        if (buf[res + i] == '\n') {
            have_read_all[num_in] = res + i + 1;
            num_in++;
        }
    }
    return;
}

void add_arg(string s) {
    call_my curr_call;
    curr_call = programs[programs.size() - 1];
    curr_call.args.push_back(s.data());
    programs[programs.size() - 1] = curr_call;
}

void parse(char* text1, int len) {
    string text = text1;
    int found_program = 0;
    programs.clear();
    int last = 0;

    if (text.substr(0, 4) == "exit") {
        exit(0);
    }

    for (int i = 0; i < len; i++) {
        if (text[i] == ' ' && last - i == 0) {
            last = i + 1;  //случай нескольких пробелов
        }
        if (i - last > 0) {
            if (text[i] == '|') {
                if (!found_program) {
                    programs.push_back(call_my(text.substr((unsigned) last, (unsigned) i - last)));
                    add_arg(text.substr((unsigned) last, (unsigned) i - last));
                } else {
                    add_arg(text.substr((unsigned) last, (unsigned) i - last));
                }
                found_program = 0;
                last = i + 1;
            }
            if (text[i] == ' ') {
                if (!found_program) {
                    programs.push_back(call_my(text.substr((unsigned) last, (unsigned) i - last)));
                    add_arg(text.substr((unsigned) last, (unsigned) i - last));
                    found_program = 1;
                } else {
                    add_arg(text.substr((unsigned) last, (unsigned) i - last));
                }
                last = i + 1;
            }
        } else {
            if (text[i] == '|') {
                found_program = 0;
                last = i + 1;
            }
        }
    }

    if (len - last > 0) {
        if (!found_program) {
            programs.push_back(call_my(text.substr((unsigned) last, (unsigned) len - last)));
            add_arg(text.substr((unsigned) last, (unsigned) len - last));
        } else {
            add_arg(text.substr((unsigned) last, (unsigned) len - last));
        }
    }
}

int* pids_global;
int pids_count;

void action(int num)
{
    for (int i = 0; i < pids_count; i++)
        kill(pids_global[i], SIGKILL);

    pids_count = 0;
}

int runpiped(call_my** programs, int n, int first_fd)
{
    if (n == 0)
        return 0;

    int pipes[n - 1][2];
    int pids[n];

    for (int i = 0; i < n - 1; i++)
    {
        int res = pipe2(pipes[i], O_CLOEXEC);

        if (res == -1)
        {
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;
        }
    }

    for (int i = 0; i < n; i++)
    {
        pids[i] = fork();

        if (pids[i] == -1)
            return -1;

        if (pids[i] != 0)
            continue;

        if (i > 0)
            dup2(pipes[i - 1][0], STDIN_FILENO);
        else {
            dup2(first_fd, STDIN_FILENO);
        }
        if (i < n - 1)
            dup2(pipes[i][1], STDOUT_FILENO);

        char* arg_[programs[i] -> args.size() + 1];
        for (int j = 0; j < programs[i]->args.size(); j++) {
            arg_[j] = (char*) programs[i]->args[j].data();
        }
        arg_[programs[i] -> args.size()] = NULL;

        int res = execvp(programs[i] -> name.data(), arg_);
        if (res == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            _exit(-1);
        }
        _exit(0);
    }

    for (int i = 0; i < n - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    pids_global = (int*) pids;
    pids_count = n;
    struct sigaction act;
    act.sa_handler = &action;

    if (sigaction(SIGINT, &act, NULL) < 0)
        return -1;

    int status;

    for (int i = 0; i < n; i++)
        waitpid(pids[i], &status, 0);

    pids_count = 0;

    return 0;
}

void nothing(int num) {}

int main(int argc, char** argv) {
    struct sigaction act;
    act.sa_handler = &nothing;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        return -1;
    }

    while(true) {

        char buff[4096];
        memset(buff, '\0', 100*sizeof(int));
        write(STDOUT_FILENO, "$ ", 2);

        if (errno != 0) {
            break;
        }

        read_until(STDIN_FILENO, buff, 4096);

        if (have_read_all[0] == 0) {
            break;
        }

        if (have_read_all[0] == -1) {
            if (write(STDOUT_FILENO, "\n", 1) == -1) {
                return -1;
            }

            errno = 0;
            continue;
        }

        parse(buff, have_read_all[0]-1);
        string input = "";
        for (int i = have_read_all[0]; buff[i] != 0; i++) {
            input += buff[i];
        }
        //cerr<<"input = "<<input<<"\n";
        int size = (int) programs.size();
        call_my* pr[size];
        call_my mas[size];
        for (int j = 0; j < size; j++) {
            mas[j] = call_my(programs[j].name, programs[j].args);
            pr[j] = &(mas[j]);
        }
        int fd[2] = {};
        pipe2(fd, O_CLOEXEC);
        write(fd[1], input.c_str(), strlen(input.c_str()));
        runpiped(pr, size, fd[0]);
        /*
        for (int i = 0; i < num_in; i++) {
            if (i > 0) {
                parse(buff + have_read_all[i - 1], have_read_all[i] - have_read_all[i - 1] - 1);
            } else {
                parse(buff, have_read_all[i] - 1);
            }
            int size = (int) programs.size();
            call_my* pr[size];
            call_my mas[size];
            for (int j = 0; j < size; j++) {
                mas[j] = call_my(programs[j].name, programs[j].args);
                pr[j] = &(mas[j]);
            }
            runpiped(pr, size);
        }*/
    }
    return 0;
}

