/************* UDP SERVER CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

// Global Variables
int udpSocket;
uint8_t inBuffer[1024];

// function declarations
int recvDataPacket(struct dataPacketHdr *hdr, uint8_t *payload, uint16_t *endOfPacket, struct sockaddr_in *clientAddr);
void sendRejectPacket(uint8_t clientId, uint16_t rejectSubCode, uint8_t recvSegNum, struct sockaddr_in *clientAddr);
void sendAckPacket(uint8_t clientId, uint8_t recvSegNum, struct sockaddr_in *clientAddr);

// main function
int main()
{
    int nBytes;
    struct sockaddr_in serverAddr; // variable to store server address
    uint8_t clientId;
    uint8_t nextSegment;            // expected segment number
    uint8_t try;                    // Client re-transmission counter
    uint8_t payloadBuffer[256];

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

// Receive 5 packets from the client
    nextSegment = 0;
    try = 0;
    while(nextSegment < 5)
    {
        struct dataPacketHdr hdr;
        uint16_t endOfPacket;
        struct sockaddr_in clientAddr;
        uint16_t recvStatus;


        recvStatus = recvDataPacket(&hdr, payloadBuffer, &endOfPacket, &clientAddr);

        if(recvStatus == 0)
        {
            // Check if segment number is expected
            if(hdr.segNum != nextSegment)
            {
                if(hdr.segNum != nextSegment - 1)
                {
                    printf("ERROR: Out of sequence - Client sent segment %d instead of %d.\n", hdr.segNum, nextSegment);

                    //Transmit reject packet (out of sequence error)
                    sendRejectPacket(clientId, REJECTION_CODE__OUT_OF_SEQUENCE, hdr.segNum, &clientAddr);

                    continue;
                }
                else
                {
                    printf("ERROR: Duplicate Packet - Client sent segment %d again.\n", hdr.segNum);

                    //Transmit reject packet (duplicate packet)
                    sendRejectPacket(clientId, REJECTION_CODE__DUPLICATE_PACKET, hdr.segNum, &clientAddr);

                    continue;
                }
            }

            if(nextSegment == 0)
            {
                clientId = hdr.clientId;
            }
/*
        packet 0: ACK is sent immediately
        packet 1: ACK is sent for the first retry
        packet 2: ACK is sent for the second retry
        packet 3: ACK is sent for the third retry
        packet 4: ACK is not sent even after third retry. The client gives up and says server is not responsive.

*/
            if((nextSegment == 1 && try < 1) ||
                (nextSegment == 2 && try < 2) ||
                (nextSegment == 3 && try < 3) ||
                (nextSegment == 4 && try < 4))
            {
                printf("Not sending ACK to client\n\n");
                try++;
                continue;
            }

            printf("Message OK. Sending ACK.\n\n");
            // Send acknowledgement packet to client
            sendAckPacket(clientId, hdr.segNum, &clientAddr);
            nextSegment++;
            try = 0;
        }
        else
        {
            // Send appropriate reject packet for packet error
            sendRejectPacket(clientId, recvStatus, hdr.segNum, &clientAddr);
        }
    }

    return 0;
}


int recvDataPacket(struct dataPacketHdr *hdr, uint8_t *payload, uint16_t *endOfPacket, struct sockaddr_in *clientAddr)
{
    uint16_t *endOfPacket1;
    uint32_t nBytes;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    // Receive a packet from the client
    nBytes = recvfrom(udpSocket, inBuffer, 1024, 0, (struct sockaddr *) clientAddr, &addrSize);

    // Copy the packet header into struct for easy decoding
    memcpy(hdr, inBuffer, sizeof(struct dataPacketHdr));


    // Decode the end-of-packet marker
    endOfPacket1 = (uint16_t *) (inBuffer + sizeof(struct dataPacketHdr) + hdr->payloadLength);
    *endOfPacket = *endOfPacket1;


    printf("Received Data Packet:\n");
    printf(" SOP: 0x%X,", hdr->startOfPacket);
    printf(" ClientID: 0x%X,", hdr->clientId);
    printf(" PktType: 0x%X,", hdr->packetType);
    printf(" SegNum: 0x%X,", hdr->segNum);
    printf(" PayLoadLen: %d,", hdr->payloadLength);

    // Check if the length field in the packet is consistent with the packet size
    if(hdr->payloadLength != nBytes - sizeof(struct dataPacketHdr) - 2)
    {
        printf("\n");
        printf("ERROR: Length mismatch\n");
        return REJECTION_CODE__LENGTH_MISMATCH;
    }

    // Copy payload in buffer to the payload destination address
    memcpy(payload, inBuffer + sizeof(struct dataPacketHdr), hdr->payloadLength);

    printf(" EOP: 0x%X\n", *endOfPacket);
    printf(" PayLoad: \"%s\"\n", payload);

    // Validate end-of-packet marker
    if(*endOfPacket != 0xFFFF)
    {
        printf("ERROR: End of packet missing\n");
        return REJECTION_CODE__END_OF_PACKET_MISSING;
    }

    // Packet has no structural errors
    return 0;
}

void sendAckPacket(uint8_t clientId, uint8_t recvSegNum, struct sockaddr_in *clientAddr)
{
    struct ackPacket ack;
    ack.startOfPacket = 0xFFFF;
    ack.clientId = clientId;
    ack.packetType = PACKET_TYPE__ACK;
    ack.recvSegNum = recvSegNum;
    ack.endOfPacket = 0xFFFF;

    sendto(udpSocket, &ack, sizeof(ack), 0, (struct sockaddr *) clientAddr, sizeof(struct sockaddr_in));
}

void sendRejectPacket(uint8_t clientId, uint16_t rejectSubCode, uint8_t recvSegNum, struct sockaddr_in *clientAddr)
{
    struct rejectPacket rej;

    rej.startOfPacket = 0xFFFF;
    rej.clientId = clientId;
    rej.packetType = PACKET_TYPE__REJECT;
    rej.rejectSubCode = rejectSubCode;
    rej.recvSegNum = recvSegNum;
    rej.endOfPacket = 0xFFFF;

    sendto(udpSocket, &rej, sizeof(rej), 0, (struct sockaddr *) clientAddr, sizeof(struct sockaddr_in));
}