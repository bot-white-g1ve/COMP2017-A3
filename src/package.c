#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/package.h>
#include <bpkg.h>
#include <utils/str.h>
#include <debug/debug.h>

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

struct btide_packet create_req_packet(const char *identifier, const char *chunk_hash, uint32_t offset, uint32_t data_len){
    /**
     * Used for REQ 0x06
    */
   struct btide_packet packet;
   packet.msg_code = 0x06;
   packet.error = 0;

   memcpy(packet.pl.data, &offset, sizeof(uint32_t));
   memcpy(packet.pl.data + sizeof(uint32_t), &data_len, sizeof(uint16_t));
   memcpy(packet.pl.data + 2*sizeof(uint32_t), chunk_hash, HASH_LEN);
   memcpy(packet.pl.data + 2*sizeof(uint32_t) + HASH_LEN, identifier, IDENTIFIER_LEN);

   return packet;
}

struct btide_packet create_res_packet(struct bpkg_obj* bpkg, const char* chunk_hash, uint32_t offset, uint16_t data_len, const char* directory) {
    struct btide_packet packet;
    packet.msg_code = 0x07;
    packet.error = 0;

    char* file_path = concat_file_path(directory, bpkg->filename);
    if (file_path == NULL) {
        packet.error = 1;
        return packet;
    }

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        d_error("create_res_packet", "Open file failed");
        free(file_path);
        packet.error = 1;
        return packet;
    }

    if (lseek(fd, offset, SEEK_SET) < 0) {
        d_error("create_res_packet", "Seek file failed");
        close(fd);
        free(file_path);
        packet.error = 1;
        return packet;
    }

    ssize_t bytes_read = read(fd, packet.pl.data + sizeof(uint32_t), data_len);
    if (bytes_read < 0) {
        d_error("create_res_packet", "Read file failed");
        close(fd);
        free(file_path);
        packet.error = 1;
        return packet;
    }

    close(fd);
    //free(file_path);

    memcpy(packet.pl.data, &offset, sizeof(uint32_t));
    memcpy(packet.pl.data + sizeof(uint32_t) + MAX_DATA_LEN, &data_len, sizeof(uint16_t));
    memcpy(packet.pl.data + sizeof(uint32_t) + MAX_DATA_LEN + sizeof(uint16_t), chunk_hash, HASH_LEN);
    memcpy(packet.pl.data + sizeof(uint32_t) + MAX_DATA_LEN + sizeof(uint16_t) + HASH_LEN, bpkg->ident, IDENT_LEN);

    d_print("create_res_packet", "the res packet is created and returned");
    return packet;
}

struct req_packet_data parse_req_packet(struct btide_packet* packet) {
    struct req_packet_data data;
    memset(&data, 0, sizeof(data));

    memcpy(&data.offset, packet->pl.data, sizeof(uint32_t));
    memcpy(&data.data_len, packet->pl.data + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(data.chunk_hash, packet->pl.data + 2*sizeof(uint32_t), HASH_LEN);
    memcpy(data.identifier, packet->pl.data + 2*sizeof(uint32_t) + HASH_LEN, IDENTIFIER_LEN);

    return data;
}
