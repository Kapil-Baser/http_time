#include "utils.h"

#if defined(_WIN32)
#include <conio.h>
#endif

void printRemoteAddress(struct addrinfo *addr);

int main(int argc, char *argv[])
{
    #if defined(_WIN32)
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data))
        {
            fprintf(stderr, "Failed to initialize.\n");
            return 1;
        }
    #endif

    // Check for proper command line arguments
    if (argc < 3)
    {
        fprintf(stderr, "Usage: tcp_client <hostname> <port>\n");
        return 1;
    }    

    printf("Configure remote address...\n");
    struct addrinfo criteria;
    memset(&criteria, 0, sizeof(criteria));
    criteria.ai_socktype = SOCK_STREAM;     // TCP
    
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &criteria, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Printing remote address...\n");
    printRemoteAddress(peer_address);

}

void printRemoteAddress(struct addrinfo *addr)
{
    char address_buffer[100];
    char service_buffer[100];
    if(getnameinfo(addr->ai_addr, addr->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST))
    {
        fprintf(stderr, "getnameinfo() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }
    printf("%s %s\n", address_buffer, service_buffer);
}