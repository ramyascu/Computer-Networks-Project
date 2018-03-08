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

void sendDataPacket(uint8_t segNum, uint8_t *payloadBuffer, uint8_t payloadLength);
int recvRespPacket();

uint8_t outBuffer[1024];
uint8_t inBuffer[1024];
uint8_t clientId = 1;
int clientSocket;
struct sockaddr_in serverAddr;

int main()
{
    uint8_t segNum;
    uint16_t msgLen;
    FILE *f = fopen("message.txt", "r");
    struct timeval timeout;
    uint8_t payloadBuffer[256];

    timeout.tv_usec = 0;
    timeout.tv_sec = 3;

    if(f == NULL)
    {
        printf("Unable to open text file message.txt\n");
        return 1;
    }

    srand(1234);

    /*Create UDP socket*/
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    for(int i = 0; i < 5; i++)
    {
        int try;
        msgLen = 1 + (rand() * 254ull) / RAND_MAX;
        fread(payloadBuffer, msgLen - 1, 1, f);

        payloadBuffer[msgLen - 1] = '\0';

        printf("Sending segment %d to server\n", i);

        for(try = 0; try < MAX_TRIES; try++)
        {
            int8_t respCode;

            sendDataPacket(i, payloadBuffer, msgLen);
            
            respCode = recvRespPacket();
            if(respCode >= 0)
            {
                break;
            }
            else
            {
                if(try < MAX_TRIES - 1)
                {
                    printf("Did not get an ack packet from host. Resending message (Try %d).\n", try + 1);
                }
                else
                {
                    printf("No response from server. Abort session\n");
                }
            }
        }

        if(try == MAX_TRIES)
            break;

        printf("\n\n");
    }

    return 0;
}

void sendDataPacket(uint8_t segNum, uint8_t *payloadBuffer, uint8_t payloadLength)
{
    struct dataPacketHdr *hdr = (struct dataPacketHdr *) outBuffer;
    uint16_t *endOfPacket;
    uint32_t nBytes;

    hdr->startOfPacket = 0xFFFF;
    hdr->clientId = clientId;
    hdr->packetType = PACKET_TYPE__DATA;
    hdr->segNum = segNum;
    hdr->payloadLength = payloadLength;

    memcpy(outBuffer + sizeof(struct dataPacketHdr), payloadBuffer, payloadLength);

    endOfPacket = (uint16_t *) (outBuffer + sizeof(struct dataPacketHdr) + payloadLength);
    *endOfPacket = 0xFFFF;

    nBytes = sizeof(struct dataPacketHdr) + payloadLength + 2;

    sendto(clientSocket, outBuffer, nBytes, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
}

int recvRespPacket()
{
    int nBytes;

    nBytes = recvfrom(clientSocket, inBuffer, 1024, 0, NULL, NULL);

    if(nBytes == -1)
    {
        printf("Timeout waiting for ACK from server\n");
        return -1;
    }
    else if(nBytes == sizeof(struct ackPacket))
    {
        struct ackPacket *ack = (struct ackPacket *) inBuffer;
        printf("Received ACK packet: SOP 0x%X, Client 0x%X, Type 0x%X, recvSegNum 0x%X, EOP 0x%X\n",
            ack->startOfPacket, ack->clientId, ack->packetType, ack->recvSegNum, ack->endOfPacket);
        return 0;
    }
    else if(nBytes == sizeof(struct rejectPacket))
    {
        return 2;
    }
    else
    {
        printf("Unknown packet!\n");
        return 1;
    }
}