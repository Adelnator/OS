#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

int flag = 0;

void on_signal_function(int signal, siginfo_t *signal_info, void *v) {
    flag = 1;
    char* recieved_signal;
    switch (signal) {
	case SIGUSR1:
	    recieved_signal = "SIGUSR1";
	    break;
	case SIGUSR2:
	    recieved_signal = "SIGUSR2";
	    break;
    }
    printf("%s", recieved_signal);
    printf(" from ");
    printf("%d\n", signal_info->si_pid);   
    exit(0);
}

int main(int argc, char *argv[]) {
    struct sigaction act = (struct sigaction) {
        .sa_flags = SA_SIGINFO,
	.sa_sigaction = on_signal_function
    };
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sleep(10);
    if (!flag) {
        printf("No signals were caught\n");
    }
    return 0;
}
