/***** COEN 233 Programming Assignment Submitted By: Ramya Padmanabhan(W1191465)*******/

/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "common.h"

#define MAX_TRIES (4)

// Function declarations
void sendAccReqPacket(struct clientDevice cd);
int recvRespPacket();
void requestAccess(struct clientDevice cd);

// Global Variables
uint8_t buffer[1024];
int clientSocket;
struct sockaddr_in serverAddr;

int main()
{
    uint16_t msgLen;
    struct timeval timeout;
    struct clientDevice cd;

    // Setting timeout value to 3 sec. This value will be used for response packet timeout.
    timeout.tv_usec = 0;
    timeout.tv_sec = 3;


    srand(1234);    // Initialize random number generator

    /*Create UDP socket*/
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    // Set socket receive timeout
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);


    printf("Case 1: clientDevice is in database and has paid\n");
    cd.technology = 4;
    cd.srcSubNum = 4085546805;
    requestAccess(cd);
    printf("\n");

    printf("Case 2: clientDevice is in database and has NOT paid\n");
    cd.technology = 3;
    cd.srcSubNum = 4086668821;
    requestAccess(cd);
    printf("\n");

    printf("Case 3: clientDevice is in database, but wrong technology\n");
    cd.technology = 4;
    cd.srcSubNum = 4086808821;
    requestAccess(cd);
    printf("\n");

    printf("Case 4: clientDevice is not in the database\n");
    cd.technology = 4;
    cd.srcSubNum = 4088011234;
    requestAccess(cd);
    printf("\n");

    return 0;
}

// Function to request access for a client device
void requestAccess(struct clientDevice cd)
{
    int trial;

    // Send an access request packet with 4 retries (in case server doesn't reply)
    for(trial = 0; trial < MAX_TRIES; trial++)
    {
        sendAccReqPacket(cd);

        if(recvRespPacket() == 0)
            break;  // received a reply from server, so don't resend packet
    }
}

// Send the access request packet to server
void sendAccReqPacket(struct clientDevice cd)
{
    struct packetHdr hdr;
    uint16_t endOfPacket = 0xFFFF;
    uint32_t nBytes;

    // Set packet header and payload length
    hdr.startOfPacket = 0xFFFF;
    hdr.clientId = 1;
    hdr.segNum = 0;
    hdr.packetType = PACKET_TYPE__ACCESS_PERMISSION;
    hdr.payloadLength = sizeof(cd);

    // Size of packet is the sum of the sizes of header, payload (client device info) and end of packet.
    nBytes = sizeof(hdr) + sizeof(cd) + sizeof(endOfPacket);

    // Copy header, payload and endOfPacket into the buffer to transmit
    memcpy(buffer, &hdr, sizeof(hdr));
    memcpy(buffer + sizeof(hdr), &cd, sizeof(cd));
    memcpy(buffer + sizeof(hdr) + sizeof(cd), &endOfPacket, sizeof(endOfPacket));

    // Send packet to server
    sendto(clientSocket, buffer, nBytes, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
}

// Receive a response packet from client
int recvRespPacket()
{
    int nBytes;
    struct packetHdr hdr;
    struct clientDevice cd;
    uint16_t endOfPacket;

    // receive a packet and place contents in buffer
    nBytes = recvfrom(clientSocket, buffer, 1024, 0, NULL, NULL);


    if(nBytes == -1)
    {
        printf("ERROR: Timeout waiting for ACK from server\n");
        return 1;
    }
    else
    {
        // Decode the packet and copy out the header, payload (client device info) and endOfPacket
        memcpy(&hdr, buffer, sizeof(hdr));
        memcpy(&cd, buffer + sizeof(hdr), sizeof(cd));
        memcpy(&endOfPacket, buffer + sizeof(hdr) + sizeof(cd), sizeof(endOfPacket));

        printf("Received response packet: SOP 0x%X, Client 0x%X, Type 0x%X, recvSegNum 0x%X, EOP 0x%X, ",
            hdr.startOfPacket, hdr.clientId, hdr.packetType, hdr.segNum, endOfPacket);

        if(nBytes == sizeof(hdr) + sizeof(cd) + sizeof(endOfPacket) &&
            hdr.payloadLength == sizeof(cd))
        {

            // Display the server's response type on the screen.
            switch(hdr.packetType)
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

                default:
                printf("Unknown packet\n");
                exit(1);
            }

            return 0;
        }
        else
        {
            printf("Bad packet. Packet size: %d, payload size %d!\n", nBytes, hdr.payloadLength);
            exit(1);
        }
    }
}