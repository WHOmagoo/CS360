// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

// Server initialization code:

int server_init(char *name)
{
    printf("==================== server init ======================\n");
    // get DOT name and IP address of this host

    printf("1 : get and show server host info\n");
    hp = gethostbyname(name);
    if (hp == 0){
        printf("unknown host\n");
        exit(1);
    }
    printf("    hostname=%s  IP=%s\n",
           hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

    //  create a TCP socket by socket() syscall
    printf("2 : create a socket\n");
    mysock = socket(AF_INET, SOCK_STREAM, 0);
    if (mysock < 0){
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with host IP and PORT# info\n");
    // initialize the server_addr structure
    server_addr.sin_family = AF_INET;                  // for TCP/IP
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
    server_addr.sin_port = 0;   // let kernel assign port

    printf("4 : bind socket to host info\n");
    // bind syscall: bind the socket to server_addr info
    r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0){
        printf("bind failed\n");
        exit(3);
    }

    printf("5 : find out Kernel assigned PORT# and show it\n");
    // find out socket port number (assigned by kernel)
    length = sizeof(name_addr);
    r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
    if (r < 0){
        printf("get socketname error\n");
        exit(4);
    }

    // show port number
    serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
    printf("    Port=%d\n", serverPort);

    // listen at port with a max. queue of 5 (waiting clients)
    printf("5 : server is listening ....\n");
    listen(mysock, 5);
    printf("===================== init done =======================\n");
}

int addNumbers(char numbers[]){
    int i = 0;

    int n1=0, n2=0, *cur = &n1;

    while(numbers[i] != '\0'){
        if(numbers[i] >= 48 && numbers[i] <= 57){
            (*cur) = (*cur) * 10 + numbers[i] - 48;
        } else if(numbers[i] == ','){
            cur = &n2;
        }

        i++;
    }

    return n1 + n2;
}


main(int argc, char *argv[])
{
    char *hostname;
    char line[MAX];

    if (argc < 2)
        hostname = "localhost";
    else
        hostname = argv[1];

    server_init(hostname);

    // Try to accept a client request
    while(1){
        printf("server: accepting new connection ....\n");

        // Try to accept a client connection as descriptor newsock
        length = sizeof(client_addr);
        client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
        if (client_sock < 0){
            printf("server: accept error\n");
            exit(1);
        }
        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
               ntohs(client_addr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop: newsock <----> client
        while(1){
            n = recv(client_sock, line, MAX, 0);
//       n = read(client_sock, line, MAX);
            if (n==0){
                printf("server: client died, server loops\n");
                close(client_sock);
                break;
            }

            // show the line string
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);

            //strcat(line, " ECHO");
            int result = addNumbers(line);
            sprintf(line, "%d", result);

            // send the echo line to client
            n = send(client_sock, line, MAX, 0);

            printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            printf("server: ready for next request\n");
        }
    }
}


