#if defined(_WIN32)
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x600
    #endif
    // Windows socket headers
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")      // This tells the Microsoft Visual C complier to link your program against the Winsock library, "ws2_32.lib"                                            
#else
    // Berkeley sockets for linux/mac
    #define _XOPEN_SOURCE 700   // Needed to add this because VS code is not picking up addrinfo struct from 'netdb' header
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>    
#endif
// Standard C headers
#include <stdio.h>
#include <string.h>
#include <time.h>

// Helper macros 
#if defined(_WIN32)
    #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
    #define CLOSESOCKET(s) closesocket(s)
    #define GETSOCKETERRNO() (WSAGetLastError())    
#else
    #define ISVALIDSOCKET(s) ((s) >= 0)
    #define CLOSESOCKET(s) close(s)
    #define SOCKET int
    #define GETSOCKETERRNO() (errno)
#endif

// Function prototype
char *getTime();

int main()
{
    // First we must initialize the win socket if we are on windows system
    #if defined(_WIN32)
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data)
        {
            fprintf(stderr, "Failed to initialize\n");
            return 1;
        }
    #endif    

    printf("Configuring local address...\n");
    // Using addrinfo to set up the criteria for our structure
    struct addrinfo criteria;
    memset(&criteria, 0, sizeof(criteria));
    criteria.ai_family = AF_INET;
    criteria.ai_socktype = SOCK_STREAM;             // Only streaming socket
    criteria.ai_flags = AI_PASSIVE;                 // Setting up this flag means that we are gonna let our server accept incoming connections from any IP range and to any available network interface to connect to.

    // This bind_address structure is filled using our requested criteria
    struct addrinfo *bind_address;
    int retVal = getaddrinfo(0, "8080", &criteria, &bind_address);
    if (retVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed Error - %s", gai_strerror(retVal));
        return 1;
    }
    
    // Creating socket
    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed, (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed, (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    freeaddrinfo(bind_address);

    printf("Listening...\n");
    int backlog = 10;                               // Number of connections that are allowed to queue up
    if (listen(socket_listen, backlog) < 0)
    {
        fprintf(stderr, "bind() failed (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client;
    socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client))
    {
        fprintf(stderr, "accept() failed (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Client is connected...\n");
    char address_buffer[INET6_ADDRSTRLEN];
    getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    printf("Reading request...\n");
    char request[1024];
    int bytes_received = recv(socket_client, request, 1024, 0);
    // Specifying the number of bytes received to be printing using the ".*" precision specifier in printf
    printf("%.*s\n", bytes_received, address_buffer);

    printf("Responding...\n");
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int response_len = strlen(response);
    int bytes_sent = send(socket_client, response, response_len, 0);
    printf("Send %d of %d bytes.\n", bytes_sent, response_len);    

    char *time_msg = getTime();
    int time_len = strlen(time_msg);
    bytes_sent = send(socket_client, time_msg, time_len, 0);
    printf("Send %d of %d bytes.\n", bytes_sent, time_len);

    printf("Closing connection...\n");
    CLOSESOCKET(socket_client);

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

    // Win socket needs to be cleaned up by calling WSACleanup() before exiting the program
    #if defined(_WIN32)
    WSACleanup();
    #endif

    
}

char *getTime()
{
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);             // Converts the time to printable string
    return time_msg;
}
