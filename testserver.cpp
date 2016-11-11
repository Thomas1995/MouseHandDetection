#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

void error(const char* msg)
{
    printf ("%s\n", msg);
    exit(1);
}
int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Usage %s <ip> <port>\n", argv[0]);
    }
    int serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock < 1) {
        error("Cannot open socket\n");
    }
    struct sockaddr_in addr;
    memset((char*)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &addr.sin_addr);

    if (bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        error("Bind error\n");
    }

    if (listen(serverSock, 5) < 0) {
        close(serverSock);
        error("Error listen\n");
    }
    printf("Server up and running\n");

    fd_set read_fds, tmp_fds;
    FD_SET(serverSock, &read_fds);
    int fdmax = serverSock;

    while (true)
    {
        tmp_fds = read_fds;
        if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
            close(serverSock);
            error("Error select");
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == serverSock) {
                    printf("Client is connecting to server\n");
                    int clientSock;
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen;
                    if ((clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrLen)) == -1) {
                        close(serverSock);
                        error("Error accept\n");
                    }
                    else {
                        if (clientSock > fdmax) {
                            fdmax = clientSock;
                        }
                        FD_SET(clientSock, &read_fds);
                        printf("Client %d connected\n", clientSock);
                    }
                }
                else {
                    int received;
                    char buffer[100];
                    memset(buffer, 0 , 100);
                    if ((received = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                        if (received == 0) {
                            printf("Client disconnected\n");
                            FD_CLR(i, &read_fds);
                            close(i);
                        }
                        else {
                            error("Recv error");
                            close(serverSock);
                        }
                    }
                    else {
                        printf("Received %s\n", buffer);

                    }
                }

            }
        }


    }

}
