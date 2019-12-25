#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <signal.h>

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT,
    ERR_FORK
};

int init_socket(const char *ip, int port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creating failed.");
        _exit(ERR_SOCKET);
    }
    struct hostent *host = gethostbyname(ip);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr, host -> h_addr_list[0],
                            sizeof(server_address.sin_addr));
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, host -> h_addr_list[0], sizeof(sin.sin_addr));
    int connect_status = connect(server_socket,
                (struct sockaddr *) &sin, sizeof(sin));
    if (connect_status < 0) {
        perror("Connection failed.");
        _exit(ERR_CONNECT);
    }
    return server_socket;
}

void print_name(char *name) {
    printf(ANSI_COLOR_BLUE "-> %s" ANSI_COLOR_RESET, name);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Incorrect arguments.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return ERR_INCORRECT_ARGS;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);
    char message[128];
    memset(message, 0, 128);
    printf("Choose a nickname (less than 48 signs):\n");
    fgets(message, 48, stdin);

    int pid[2];
    pid[0] = fork();
    if (pid[0] < 0) {
        perror("Forking failed.");
        _exit(ERR_FORK);
    }
    if (!pid[0]) {
        while (1) {
            read(server, message, 128);
            if (!strcmp(&message[48], "-exit\n"))
                return OK;
            print_name(message);
            fputs(&message[48], stdout);
        }
    }
    pid[1] = fork();
    if (pid[1] < 0) {
        perror("Forking failed.");
        _exit(ERR_FORK);
    }
    if (!pid[1]) {
        while (1) {
            fgets(&message[48], 80, stdin);
            write(server, message, 128);
            if (!strcmp(&message[48], "-exit\n"))
                return OK;
        }
    }
    wait(NULL);
    wait(NULL);
    close(server);
    return OK;
}
