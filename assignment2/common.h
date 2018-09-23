// Common data-structures for both client and server

struct __attribute__((__packed__)) packetHdr
{
    uint16_t startOfPacket;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t segNum;
    uint8_t payloadLength;
};

struct __attribute__((__packed__)) clientDevice
{
    uint8_t technology;
    uint32_t srcSubNum;
};

enum TECHNOLOGY
{
    TECH_2G = 2,
    TECH_3G = 3,
    TECH_4G = 4,
    TECH_5G = 5,
};

enum PACKET_TYPE
{
    PACKET_TYPE__ACCESS_PERMISSION = 0xFFF8,
    PACKET_TYPE__NOT_PAID,
    PACKET_TYPE__NOT_EXIST,
    PACKET_TYPE__ACCESS_OK,
};
