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

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN,
    ERR_FORK
};

int init_socket(int port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creating failed.");
        _exit(ERR_SOCKET);
    }
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                            &socket_option, sizeof(socket_option));
    if (server_socket < 0) {
        perror("Setting socket options failed.");
        _exit(ERR_SETSOCKETOPT);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int bind_status = bind(server_socket, (struct sockaddr *) &server_address,
                                                    sizeof(server_address));
    if (bind_status < 0) {
        perror("Binding socket address failed.");
        _exit(ERR_BIND);
    }
    int listen_status = listen(server_socket, 4);
    if (listen_status < 0) {
        perror("Bind socket address failed.");
        _exit(ERR_LISTEN);
    }
    return server_socket;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("Incorrect arguments.");
        puts("./server <port>");
        puts("Example:");
        puts("./server 5000");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    int server_socket = init_socket(port);
    struct sockaddr_in client_address[64];
    socklen_t size[64];
    int client_socket[64];
    int *online = mmap(NULL, 64 * sizeof(int), PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(online, 0, 64 * sizeof(int));
    char message[128];
    int pid1[64], pid2[64];
    int fd[64][2];
    for (int i = 0; i < 64; i++)
        pipe(fd[i]);
    for (int i = 0; i < 64; i++) {
        client_socket[i] = accept(server_socket,
                        (struct sockaddr *) &client_address[i], &size[i]);
        printf("Connected: %s %d\n", inet_ntoa(client_address[i].sin_addr),
                                        ntohs(client_address[i].sin_port));
        online[i] = 1;
        pid1[i] = fork();
        if (pid1[i] < 0) {
            perror("Forking error.\n");
            _exit(ERR_FORK);
        }
        if (!pid1[i]) {
            while (1) {
                read(fd[i][0], message, 128);
                write(client_socket[i], message, 128);
            }
        }
        pid2[i] = fork();
        if (pid2[i] < 0) {
            perror("Forking error.\n");
            _exit(ERR_FORK);
        }
        if (!pid2[i]) {
            while (1) {
                read(client_socket[i], message, 128);
                if (!strcmp(&message[48], "-exit\n")) {
                    write(client_socket[i], message, 128);
                    online[i] = 0;
                    kill(pid1[i], SIGKILL);
                    return OK;
                } else {
                    for (int j = 0; j < 64; j++) {
                        if (i != j && online[j])
                            write(fd[j][1], message, 128);
                    }
                }
            }
        }
    }
    printf("Chat supports only 64 unique connections.\n");
    for (int i = 0; i < 128; i++)
        wait(NULL);
    return OK;
}
