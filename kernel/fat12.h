#ifndef KERNEL_FAT12_H
#define KERNEL_FAT12_H

#include <stdint.h>
#include "memory.h"

#define FAT12_SIGNATURE 0xAA55

struct fat12_boot_sector {
    uint8_t jump[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t extended_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    uint16_t signature;
} __attribute__((packed));

struct fat12_dir_entry {
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed));

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

int fat12_init(void);
int fat12_read_file(const char *filename, uint8_t *buffer, uint32_t *size);
int fat12_write_file(const char *filename, const uint8_t *data, uint32_t size);
int fat12_list_dir(void);

#endif