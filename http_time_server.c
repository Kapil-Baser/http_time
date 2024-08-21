#if defined(_WIN32)
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x600
    #endif
    // Windows socket headers
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")      // If compiling using visual studio compiler
#else
    // Berkley sockets for linux/mac
    #define _XOPEN_SOURCE 700   // Needed to add this because VS code is not picking up addrinfo struct from netdb.h.
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
        fprintf(stderr, "getaddrinfo() failed", gai_strerror(retVal));
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
}