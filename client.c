// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;
struct sockaddr_in  server_addr;

int server_sock, r;
int SERVER_IP, SERVER_PORT;

char stringBuf[MAX];

// clinet initialization code

int client_init(char *argv[])
{
    printf("======= clinet init ==========\n");

    printf("1 : get server info\n");
    hp = gethostbyname(argv[1]);
    if (hp==0){
        printf("unknown host %s\n", argv[1]);
        exit(1);
    }

    SERVER_IP   = *(long *)hp->h_addr;
    SERVER_PORT = atoi(argv[2]);f

    printf("2 : create a TCP socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock<0){
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with server's IP and PORT#\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = SERVER_IP;
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to server
    printf("4 : connecting to server ....\n");
    r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0){
        printf("connect failed\n");
        exit(1);
    }

    printf("5 : connected OK to \007\n");
    printf("---------------------------------------------------------\n");
    printf("hostname=%s  IP=%s  PORT=%d\n",
           hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
    printf("---------------------------------------------------------\n");

    printf("========= init done ==========\n");
}

sendPut(char *fileName){
    FILE * file = fopen(fileName, "r");

    if(file) {
        sendString(fileName, MAX);

        while (!feof(file)) {
            char data[MAX] = "\0";

            fread(data, 1, MAX - 1, file);

            sendString(data, MAX);
        }

        sendString("\0", MAX);
    }
}

receiveGet(char *fileName){
    FILE *file = fopen(fileName, "w");

    if(file) {

        sendString(fileName, MAX);

        while(1) {
            receiveString();

            if(strcmp(stringBuf, "\0") == 0){
                break;
            }

            fprintf(file, "%s", stringBuf);

        }

        fclose(file);

    } else {
        printf("Could not open %s for writing\n", stringBuf);
    }
}

int receiveString(){
    int n;
    n = recv(server_sock, stringBuf, MAX, 0);
    n = send(server_sock, "ok", 4, 0);

    return 1;

}

int sendString(char string[], int length){
    char response[4];
    int n = send(server_sock, string, length, 0);
    n = recv(server_sock, response, 4, 0);

    if(strcmp(response, "ok") == 0) {
        return 1;
    } else{
        printf("There was an error while sending the string.\nServer reponse was: %s", response);
        return 0;
    }
}

main(int argc, char *argv[ ])
{
    int n;
    char line[MAX], ans[MAX];

    if (argc < 3){
        printf("Usage : client ServerName SeverPort\n");
        exit(1);
    }

    client_init(argv);
    // sock <---> server
    printf("********  processing loop  *********\n");
    while (1){
        printf("input a line : ");
        bzero(line, MAX);                // zero out line[ ]
        fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

        line[strlen(line)-1] = 0;        // kill \n at end
        if (line[0]==0) {                  // exit if NULL line
            exit(0);
        }



        char *command;
        char *args;

        command = strtok(line, " ");

        if(sendString(command, 16)){
            if(strcmp(command, "put") == 0){
                char *file = strtok(NULL, " ");
                sendPut(file);
            } else if(strcmp(command, "get") == 0){
                char *file = strtok(NULL, " ");
                receiveGet(file);
            }
        }
        //n = send(server_sock, line, MAX, 0);


        // Send ENTIRE line to server
        //n = write(server_sock, line, MAX);

//        sendPut("something");
//
//        n = send(server_sock, line, MAX, 0);
//
//        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
//
//        // Read a line from sock and show it
//        printf("waiting to recieve\n");
//        n = recv(server_sock, ans, MAX, 0);
//        printf("recieved");
//
////        n = read(server_sock, ans, MAX);
//        printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
    }
}



