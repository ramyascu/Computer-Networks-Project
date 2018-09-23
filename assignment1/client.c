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

#define MAX_TRIES (4) // First transmission + 3 re-try's

//function declaration
void sendDataPacket(uint8_t segNum, uint8_t *payloadBuffer, uint8_t payloadLength);
uint16_t recvRespPacket();

// global variables
uint8_t outBuffer[1024];
uint8_t inBuffer[1024];
uint8_t clientId = 1;
int clientSocket;
struct sockaddr_in serverAddr;

//main function
int main()
{
    uint8_t segNum;
    uint16_t msgLen;
    FILE *f = fopen("message.txt", "r"); // open a .txt file which will act as payload
    struct timeval timeout;
    uint8_t payloadBuffer[256];

// Ack timer is set to 3 seconds
    timeout.tv_usec = 0;
    timeout.tv_sec = 3;

    if(f == NULL)
    {
        printf("Unable to open text file message.txt\n");
        return 1;
    }

    srand(1234); //initialize random no. generator

    /*Create UDP socket*/
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    
    // send 5 data packets to the server and receive ACK
    for(int i = 0; i < 5; i++)
    {
        int try; //packet retransmission counter

        // Read a random no. of characters from a file (1-255 characters(bytes))
        msgLen = 1 + (rand() * 254ull) / RAND_MAX;
        fread(payloadBuffer, msgLen - 1, 1, f); // read msg length - 1 characters from the file into payload buffer

        payloadBuffer[msgLen - 1] = '\0'; //null termination of the string

        printf("Sending segment %d to server\n", i);

        for(try = 0; try < MAX_TRIES; try++) // make MAX_TRIES attempts to send data packet and receive ACK
        {
            uint16_t respCode;

            sendDataPacket(i, payloadBuffer, msgLen);
            
            respCode = recvRespPacket(); // receive response packet with time out of 3 sec
            if(respCode == 0) // Successful transmission with ACK received, break out of re-try loop
            {
                break;
            }
            else if(respCode < 0xFFFF) // if got a rejection packet. terminate client.
            {
                exit(1);
            }
            else
            {
                //respCode= 0xFFFF indicates ACK timer expired. So, either retransmit or abort session if try==MAX_TRIES
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
            break;          // Packet retransmission limit reached. Don't send anymore packets.

        printf("\n\n");
    }

    return 0;
}

// Send data packet with payload
void sendDataPacket(uint8_t segNum, uint8_t *payloadBuffer, uint8_t payloadLength)
{
    struct dataPacketHdr *hdr = (struct dataPacketHdr *) outBuffer; // get pointer to header region of piont buffer
    uint16_t *endOfPacket;
    uint32_t nBytes;

    //set the header
    hdr->startOfPacket = 0xFFFF;
    hdr->clientId = clientId;
    hdr->packetType = PACKET_TYPE__DATA;
    hdr->segNum = segNum;
    hdr->payloadLength = payloadLength;

    // copy the payloadBuffer into packet buffer.
    memcpy(outBuffer + sizeof(struct dataPacketHdr), payloadBuffer, payloadLength);
    // get pointer to the end of packet field in the packet buffer.
    endOfPacket = (uint16_t *) (outBuffer + sizeof(struct dataPacketHdr) + payloadLength);
    *endOfPacket = 0xFFFF;

    // set the packet size
    nBytes = sizeof(struct dataPacketHdr) + payloadLength + 2;

    sendto(clientSocket, outBuffer, nBytes, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
}

// Receive response packet with timeout
uint16_t recvRespPacket()
{
    int nBytes;
    // receive a packet from server, the size of the packet is nBytes.
    nBytes = recvfrom(clientSocket, inBuffer, 1024, 0, NULL, NULL);

    // Did not receive packet within the timeout.
    if(nBytes == -1)
    {
        printf("Timeout waiting for ACK from server\n");
        return 0xFFFF;
    }

    else if(nBytes == sizeof(struct ackPacket))
    {
        // Received ACK packet
        struct ackPacket *ack = (struct ackPacket *) inBuffer;
        printf("Received ACK packet: SOP 0x%X, Client 0x%X, Type 0x%X, recvSegNum 0x%X, EOP 0x%X\n",
            ack->startOfPacket, ack->clientId, ack->packetType, ack->recvSegNum, ack->endOfPacket);
        return 0;
    }
    else if(nBytes == sizeof(struct rejectPacket))
    {   
        // Received Reject packet
        struct rejectPacket *rej = (struct rejectPacket *) inBuffer;
        printf("Received REJECT packet: SOP 0x%X, Client 0x%X, Type 0x%X, rejectSubCode 0x%X, recvSegNum 0x%X, EOP 0x%X\n",
            rej->startOfPacket, rej->clientId, rej->packetType, rej->rejectSubCode, rej->recvSegNum, rej->endOfPacket);
        return rej->rejectSubCode;
    }
    else
    {
        printf("Unknown packet!\n");
        exit(1);
    }
}