#include "../include/client.h"


int connect_to_server(const char *host, const uint16_t port){
    
    struct sockaddr_in server_addr;

int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

if (socket_fd < 0) {
        perror("socket");
        exit(1);
    }

     memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

     if (connect(client_fd, (struct sockaddr *)&s, sizeof(server_addr)) < 0) {
        perror("connect");
        close(socket_fd);
    }

    return socket_fd;
}
int main(void) {
   
    int socket_fd = connect_to_server(SERVER_IP, PORT);

    if (socket_fd < 0) {
        printf("Failed to connect to server\n");
        return 1;
    }

        printf("Connected to server\n");

        close(socket_fd);

    return 0; 
}