#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_DATA 20480

char message[MAX_DATA];
void *modules[256];
int mod_count = 0;
ssize_t bytes_recv;
int should_exit = 0;
void *handle;
int client_sock;

void *do_load(char *module){
    void *handle = dlopen(module, RTLD_NOW);

    if(handle != NULL){
        for(int i = 0; i < mod_count; i++){
            if(modules[i] == handle){
                return handle;
            }
        }

        modules[mod_count] = handle;
        mod_count++;
        return handle;
    }

    fprintf(stderr, "Error: %s\n", dlerror());
    return handle;
}

int do_modinfo(char *buff){
    FILE *fp = fopen("/proc/self/maps", "r");

    if(fp == NULL){
        return 0;
    }

    fread(buff, 1, MAX_DATA-1, fp);
    fclose(fp);

    return 1;
}

void handle_conn(){
    char buff[128];

    while(1){
        bytes_recv = recv(client_sock, buff, MAX_DATA, 0);
        printf("Received %d bytes\n", bytes_recv);

        if(bytes_recv == 0 || bytes_recv == -1){
            break;
        }

        if(buff[0] == '\r' || buff[0] == '\n'){
            continue;
        }

        if(strncmp(buff, "modinfo", 7) == 0){
            if(!do_modinfo(message)){
                sprintf(message, "Failed to get modinfo\n");
            }
        }
        else if(strncmp(buff, "load", 4) == 0){
            for(int i = 0; i < bytes_recv; i++){
                if(buff[i] == '\n' || buff[i] == '\r'){
                    buff[i] = '\0';
                    break;
                }
            }

            if(bytes_recv <= 5){
                sprintf(message, "You must provide a module name or path\n");
            }
            else if(handle = do_load(&buff[5])){
                sprintf(message, "Module %s was loaded at %p\n", &buff[5], handle);
            }
            else{
                sprintf(message, "Failed to load %s\n", &buff[5]);
            }
        }
        else if(strncmp(buff, "unload", 6) == 0){
            sprintf(message, "");
            while(mod_count != 0){
                sprintf(message, "%sUnloaded %p\n", message, modules[mod_count-1]);
                dlclose(modules[mod_count-1]);
                mod_count--;
            }

            sprintf(message, "%sDone\n", message);
        }
        else if(strncmp(buff, "quit", 4) == 0){
            sprintf(message, "Bye\n");
            should_exit = 1;
        }
        else if(strncmp(buff, "help", 4) == 0 || buff[0] == '?'){
            sprintf(message, "Available commands:\n");
            sprintf(message, "%s\tmodinfo\n\tload\n\tunload\n\tquit\n\thelp\n", message);
        }
        else{
            sprintf(message, "Unknown command\n");
        }

        send(client_sock, message, strlen(message), 0);

        if(should_exit){
            break;
        }
    }
}

int main(int argc, char *argv[]){
    int fd, fdclient, reuseaddr;
    socklen_t len;
    struct sockaddr_in server = {0};
    struct sockaddr_in client = {0};

    if(argc != 2){
        printf("Usage: %s <listen-addr>\n", argv[0]);
        printf("Examples:\n");
        printf("\t%s 127.0.0.1\n", argv[0]);
        printf("\t%s 0.0.0.0\n", argv[0]);
        return 1;
    }

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket()");
        return 1;
    }

    reuseaddr = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
        perror("setsockopt()");
        close(fd);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(1337);
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if(bind(fd, (struct sockaddr *) &server, sizeof(server)) != 0){
        perror("bind()");
        close(fd);
        return 1;
    }

    if(listen(fd, 5) != 0){
        perror("listen()");
        close(fd);
        return 1;
    }

    printf("Listening on %s:1337\n", argv[1]);
    
    while(1){
        len = sizeof(struct sockaddr_in);
        fdclient = accept(fd, (struct sockaddr *) &client, &len);

        if(fdclient < 0){
            printf("fdclient %d\n", fdclient);
            perror("Failed to accept\n");
            continue;
        }

        printf("Client connected\n");
        client_sock = fdclient;

        handle_conn();
        close(client_sock);
    }
}