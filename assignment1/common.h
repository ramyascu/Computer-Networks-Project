// This is a common data structures for both client and server.

struct __attribute__((__packed__)) dataPacketHdr
{
    uint16_t startOfPacket;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t segNum;
    uint8_t payloadLength;
};

struct __attribute__((__packed__)) ackPacket
{
    uint16_t startOfPacket;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t recvSegNum;
    uint16_t endOfPacket;
};

struct __attribute__((__packed__)) rejectPacket
{
    uint16_t startOfPacket;
    uint8_t clientId;
    uint16_t packetType;
    uint16_t rejectSubCode;
    uint8_t recvSegNum;
    uint16_t endOfPacket;
}; 


enum REJECTION_CODE{
    REJECTION_CODE__OUT_OF_SEQUENCE=0xFFF4,
    REJECTION_CODE__LENGTH_MISMATCH,
    REJECTION_CODE__END_OF_PACKET_MISSING,
    REJECTION_CODE__DUPLICATE_PACKET,
};

enum PACKET_TYPE
{
    PACKET_TYPE__DATA = 0xFFF1,
    PACKET_TYPE__ACK,
    PACKET_TYPE__REJECT,
};
