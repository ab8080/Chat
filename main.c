#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pthread.h>

volatile sig_atomic_t return0_flag = 0;
volatile sig_atomic_t server_fd;
volatile sig_atomic_t client_fd[2];

void exit_handler() {
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    shutdown(client_fd[0], SHUT_RDWR);
    close(client_fd[0]);
    shutdown(client_fd[1], SHUT_RDWR);
    close(client_fd[1]);
    return0_flag = 1;

}

struct history {
    int sz;
    int capacity;
    char* arr;
    int client_state;
    int master_state;
};

void* master_func(void* args) {
    struct history* h = (struct history*)args;

    while (!return0_flag) {
        ssize_t bytes_read;
        char buf[2048];
        bytes_read = read(client_fd[0], buf, 2048);
        if (bytes_read <= 0) {
            h->master_state = 0;
            const char* msg = "Client has disconnected but you can still send messages\n"
                        "He will recieve them when he connects\n";
            write(client_fd[1], msg, strlen(msg));
            client_fd[0] = accept(server_fd, NULL, NULL);

            if (client_fd[0] == -1) {
                return NULL;
            }
            h->master_state = 1;
            write(client_fd[0], "History:\n", strlen("History:\n"));

            write(client_fd[0], h->arr, h->sz);
            continue;
        }

        while (h->sz + bytes_read + sizeof "master: " > h->capacity) {
            h->arr = (char *) realloc(h->arr, h->capacity * 2);
            h->capacity *= 2;
        }

        strcat(h->arr, "master: ");
        strncat(h->arr, buf, bytes_read);
        h->sz += bytes_read + sizeof("master: ");

        if (h->client_state == 1) {
            write(client_fd[1], buf, bytes_read);
        }
    }
    return NULL;
}

void* client_func(void* args) {
    struct history* h = (struct history*)args;

    while (!return0_flag) {
        ssize_t bytes_read;
        char buf[2048];
        bytes_read = read(client_fd[1], buf, 2048);
        if (bytes_read <= 0) {
            h->client_state = 0;
            const char* msg = "Client has disconnected but you can still send messages\n"
                              "He will recieve them when he connects\n";
            write(client_fd[0], msg, strlen(msg));
            client_fd[1] = accept(server_fd, NULL, NULL);

            if (client_fd[1] == -1) {
                return NULL;
            }
            h->client_state = 1;
            write(client_fd[1], "History:\n", sizeof("History:\n"));

            write(client_fd[1], h->arr, h->sz);
            continue;
        }
        while (h->sz + bytes_read + sizeof "client: " > h->capacity) {
            h->arr = (char *) realloc(h->arr, h->capacity * 2);
            h->capacity *= 2;
        }

        strcat(h->arr, "client: ");
        strncat(h->arr, buf, bytes_read);
        h->sz += bytes_read + sizeof "client: ";

        if (h->master_state == 1) {
            write(client_fd[0], buf, bytes_read);
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = exit_handler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);

    const char* ip_str = argv[1];
    const char* port_str = argv[2];

    uint16_t port = htons(strtol(port_str, NULL, 10));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adr;

    inet_pton(AF_INET, ip_str, &adr.sin_addr);
    adr.sin_family = AF_INET;
    adr.sin_port = port;

    bind(server_fd, (struct sockaddr *) &adr, sizeof(adr));
    listen(server_fd, 128);

    pthread_t master;
    pthread_t client;

    struct history arg;
    arg.sz = 0;
    arg.capacity = 100;
    arg.arr = (char *)malloc(100);
    arg.client_state = 1;
    arg.master_state = 1;

    for (int i = 0; i < 2; ++i) {
        client_fd[i] = accept(server_fd, NULL, NULL);
        if (client_fd[i] == -1) {
            return 0;
        }
    }

    pthread_create(&master, NULL, &master_func, (void*)&arg);
    pthread_create(&client, NULL, &client_func, (void*)&arg);

    pthread_join(master, NULL);
    pthread_join(client, NULL);
    arg.capacity = 0;
    free(arg.arr);
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    return 0;
}