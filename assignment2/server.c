/***** COEN 233 Programming Assignment Submitted By: Ramya Padmanabhan(W1191465)*******/

/************* UDP SERVER CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

int udpSocket;
uint8_t buffer[1024];
uint8_t buffer[1024];

// Function declaration
void sendPacket(struct packetHdr hdr,struct clientDevice cd, struct sockaddr_in *clientAddr);
int recvPacket(struct packetHdr *hdr,struct clientDevice *cd, struct sockaddr_in *clientAddr);
uint16_t clientDevLookup(struct clientDevice cd);


//main function
int main()
{
    int nBytes;
    struct sockaddr_in serverAddr;
    uint8_t clientId;
    struct packetHdr hdr;
    struct clientDevice cd;

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    // Receives access request form client and send appropriate response.
    while(1)
    {
        struct packetHdr hdr;
        struct sockaddr_in clientAddr;
        uint16_t response;


        recvPacket(&hdr, &cd, &clientAddr);

        // Lookup client device in database
        response = clientDevLookup(cd);

        switch(response)
        {
            case PACKET_TYPE__ACCESS_OK:
            printf("Access OK\n");
            break;

            case PACKET_TYPE__NOT_PAID:
            printf("Access denied - not paid\n");
            break;

            case PACKET_TYPE__NOT_EXIST:
            printf("Access denied - Subscriber does not exist\n");
            break;
        }
        printf("\n");

        hdr.packetType = response;

        // Send acknowledgement packet to client
        sendPacket(hdr, cd, &clientAddr);
        
    }

    return 0;
}


int recvPacket(struct packetHdr *hdr, struct clientDevice *cd, struct sockaddr_in *clientAddr)
{
    uint16_t *endOfPacket;
    uint32_t nBytes;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    // Receive a packet from the client
    nBytes = recvfrom(udpSocket, buffer, 1024, 0, (struct sockaddr *) clientAddr, &addrSize);

    // Copy the packet header into struct for easy decoding
    memcpy(hdr, buffer, sizeof(struct packetHdr));


    // Decode the end-of-packet marker
    endOfPacket = (uint16_t *) (buffer + sizeof(struct packetHdr) + hdr->payloadLength);

    printf("Received Packet:");
    printf(" SOP: 0x%X,", hdr->startOfPacket);
    printf(" ClientID: 0x%X,", hdr->clientId);
    printf(" PktType: 0x%X,", hdr->packetType);
    printf(" SegNum: 0x%X,", hdr->segNum);
    printf(" PayLoadLen: %d,", hdr->payloadLength);

    // Check if the length field in the packet is consistent with the packet size
    if(hdr->payloadLength != nBytes - sizeof(struct packetHdr) - 2 || 
        hdr->payloadLength != sizeof(struct clientDevice))
    {
        printf("\n");
        printf("ERROR: Length mismatch\n");
        exit(1);
    }

    // Copy payload in buffer to the payload destination address
    memcpy(cd, buffer + sizeof(struct packetHdr), hdr->payloadLength);

    printf(" Subscriber: %u,", cd->srcSubNum);
    printf(" Technology: %d,", cd->technology);
    printf(" EOP: 0x%X\n", *endOfPacket);

    // Validate end-of-packet marker
    if(*endOfPacket != 0xFFFF)
    {
        printf("ERROR: End of packet missing\n");
        exit(1);
    }

    // Packet has no structural errors
    return 0;
}

void sendPacket(struct packetHdr hdr, struct clientDevice cd, struct sockaddr_in *clientAddr)
{
    uint16_t endOfPacket = 0xFFFF;

    hdr.startOfPacket = 0xFFFF;
    hdr.payloadLength = sizeof(cd);

    memcpy(buffer, &hdr, sizeof(hdr));
    memcpy(buffer + sizeof(hdr), &cd, sizeof(cd));
    memcpy(buffer + sizeof(hdr) + sizeof(cd), &endOfPacket, sizeof(endOfPacket));

    sendto(udpSocket, buffer, sizeof(hdr) + sizeof(cd) + sizeof(endOfPacket), 0, 
        (struct sockaddr *) clientAddr, sizeof(struct sockaddr_in));
}

uint16_t clientDevLookup(struct clientDevice cd)
{
    unsigned long int subNum;
    char subNumStr[13];
    int tech;
    int paid;
    FILE *f = fopen("Verification_Database.txt", "r");
    
    if(f == NULL)
    {
        printf("ERROR: Unable to open database file\n");
        exit(1);
    }

    fscanf(f, "%*[^\n]\n", NULL);

    while(fscanf(f, "%12s %d %d", subNumStr, &tech, &paid) == 3)
    {
        int i;
        subNum = 0;
        for(i = 0; i < strlen(subNumStr); i++)
        {
            if(subNumStr[i] >= '0' && subNumStr[i] <= '9')
            {
                subNum *= 10;
                subNum += subNumStr[i] - '0';                
            }
        }

        // printf("subNum: %lu, tech: %d, paid: %d\n", subNum, tech, paid);

        if(cd.srcSubNum == subNum && cd.technology == tech)
        {
            if(paid)
            {
                fclose(f);
                return PACKET_TYPE__ACCESS_OK;
            }
            else
            {
                fclose(f);
                return PACKET_TYPE__NOT_PAID;
            }
        }
    }

    fclose(f);
    return PACKET_TYPE__NOT_EXIST;
}
