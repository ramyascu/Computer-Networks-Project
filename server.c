/************* UDP SERVER CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

struct __attribute__((__packed__)) dataPacketHdr{
  uint16_t startOfPacket;
  uint8_t clientId;
  uint16_t packetType;
  uint8_t segNum;
  uint8_t payloadLength;
};

enum REJECTION_CODE{
  REJECTION_CODE__OUT_OF_SEQUENCE=0xFFF4,
  REJECTION_CODE__LENGTH_MISMATCH,
  REJECTION_CODE__END_OF_PACKET_MISSING,
  REJECTION_CODE__DUPLICATE_PACKET,
};

int udpSocket;
char inBuffer[1024];
char outBuffer[16];
char payloadBuffer[256];

socklen_t addr_size;
int i;


int main(){

  int nBytes;
  struct sockaddr_in serverAddr;
  uint8_t clientId;
  uint8_t nextSegment;

  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  nextSegment = 0;
  while(1){
    struct dataPacketHdr hdr;
    uint16_t endOfPacket;
    struct sockaddr_in clientAddr;
    uint16_t recvStatus;


    recvStatus = recvPacket(&hdr, payloadBuffer, &endOfPacket, &clientAddr);

    if(recvStatus == 0)
    {
      // Check if segment number is expected
      if(hdr->segNum != nextSegment)
      {
        printf("ERROR: Server expected %d segment, but received %d segment\n", nextSegment, hdr->segNum);

        // Transmit error 
      }
      if(nextSegment == 0)
      {
        if(hdr->segNum == 0)
      }

      // Send acknowledgement packet to client.
    }

    /*Send uppercase message back to client, using serverStorage as the address*/
    sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
  }


  return 0;
}


int recvPacket(struct dataPacketHdr *hdr, uint8_t *payload, uint16_t *endOfPacket, struct sockaddr_in *clientAddr)
{
  uint16_t *endOfPacket1;
  socklen_t addrSize = sizeof(struct sockaddr_in);

  // Receive a packet from the client
  nBytes = recvfrom(udpSocket, inBuffer, 1024, 0, (struct sockaddr *) clientAddr, &addrSize);

  // Copy the packet header into struct for easy decoding
  memcpy(hdr, inBuffer, sizeof(struct dataPacketHdr));

  // Check if the length field in the packet is consistent with the packet size
  if(hdr->payloadLength != nBytes - sizeof(struct dataPacketHdr) - 2)
    return 1;

  // Decode the end-of-packet marker
  endofPacket1 = inBuffer + sizeof(struct dataPacketHdr) + hdr->payloadLength;
  *endOfPacket = *endOfPacket1;

  // Validate end-of-packet marker
  if(*endOfPacket != 0xFFFF)
    return 2;

  // Copy payload in buffer to the payload destination address
  memcpy(payload, inBuffer + sizeof(struct dataPacketHdr), hdr->payloadLength);

  // Packet has no structural errors
  return 0;
}
