#include <net/package.h>
#include <string.h>

struct btide_packet create_small_packet(uint16_t msg_code){
    /**
     * Userd for:
     * ACP 0x02
     * ACK 0x0c
     * DSN 0x03
     * PNG 0xFF
     * POG 0x00
    */
    struct btide_packet packet;
    packet.msg_code = msg_code;
    packet.error = 0;
    memset(packet.pl.data, 0, PAYLOAD_MAX);
    return packet;
}

struct btide_packet create_req_packet(const char *identifier, const char *chunk_hash, uint32_t offset, uint16_t data_len){
    /**
     * Used for REQ 0x06
    */
   struct btide_packet packet;
   packet.msg_code = 0x06;
   packet.error = 0;

   memcpy(packet.pl.data, &offset, sizeof(uint32_t));
   memcpy(packet.pl.data + sizeof(uint32_t), &data_len, sizeof(uint16_t));
   memcpy(packet.pl.data + sizeof(uint32_t) + sizeof(uint16_t), chunk_hash, HASH_LEN);
   memcpy(packet.pl.data + sizeof(uint32_t) + sizeof(uint16_t) + HASH_LEN, identifier, IDENTIFIER_LEN);

   return packet;
}

struct btide_packet create_res_packet();
// TODO