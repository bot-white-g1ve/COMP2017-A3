#ifndef NETPKT_H
#define NETPKT_H

#include <stdint.h>
#include <bpkg.h>

#define PAYLOAD_MAX (4092)
#define IDENTIFIER_LEN (1024)
#define HASH_LEN (64)
#define MAX_DATA_LEN (2998)

#define PKT_MSG_ACK 0x0c
#define PKT_MSG_ACP 0x02
#define PKT_MSG_DSN 0x03
#define PKT_MSG_REQ 0x06
#define PKT_MSG_RES 0x07
#define PKT_MSG_PNG 0xFF
#define PKT_MSG_POG 0x00


union btide_payload {
    uint8_t data[PAYLOAD_MAX];

};

struct btide_packet {
    uint16_t msg_code;
    uint16_t error;
    union btide_payload pl;
};

struct req_packet_data {
    char identifier[IDENTIFIER_LEN];
    char chunk_hash[HASH_LEN];
    uint32_t offset;
    uint32_t data_len;
};

extern struct btide_packet create_small_packet(uint16_t msg_code);
extern struct btide_packet create_req_packet(const char *identifier, const char *chunk_hash, uint32_t offset, uint32_t data_len);
extern struct btide_packet create_res_packet(struct bpkg_obj* bpkg, const char* chunk_hash, uint32_t offset, uint16_t data_len, const char* directory);
extern struct req_packet_data parse_req_packet(struct btide_packet* packet);

#endif
