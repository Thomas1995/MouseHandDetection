#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "include/DataProcessor.h"
#include "client.h"

int serverSock;

void error(const char *msg)
{
    printf("%s\n", msg);
    exit(0);
}
void connect_to_server(int argc, char* argv[])
{
    if (argc != 3) {
        error("Usage %s ip port\n");
    }
    struct sockaddr_in to_station;
    serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock < 1) {
        error("Error opening socket\n");
    }
    memset((char*)&to_station, 0, sizeof(to_station));

    to_station.sin_family = AF_INET;
    to_station.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &to_station.sin_addr);

    if (connect(serverSock, (struct sockaddr*)&to_station, sizeof(to_station)) < 0) {
        error("Error connecting\n");
    }

    printf("Client connected to server %d\n", serverSock);
}
void identify()
{
    char team_name[7] = "KREST\0";
    send(serverSock, team_name, 6, 0);
}
