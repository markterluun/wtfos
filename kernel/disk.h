#ifndef KERNEL_DISK_H
#define KERNEL_DISK_H

#include <stdint.h>
#include <stdbool.h>

// IDE ports
#define IDE_DATA        0x1F0
#define IDE_ERROR       0x1F1
#define IDE_SECTOR_COUNT 0x1F2
#define IDE_SECTOR_NUM  0x1F3
#define IDE_CYL_LOW     0x1F4
#define IDE_CYL_HIGH    0x1F5
#define IDE_DRIVE_HEAD  0x1F6
#define IDE_STATUS      0x1F7
#define IDE_COMMAND     0x1F7

// Status bits
#define IDE_STATUS_ERR  0x01
#define IDE_STATUS_DRQ  0x08
#define IDE_STATUS_SRV  0x10
#define IDE_STATUS_DF   0x20
#define IDE_STATUS_RDY  0x40
#define IDE_STATUS_BSY  0x80

// Commands
#define IDE_CMD_READ    0x20
#define IDE_CMD_WRITE   0x30
#define IDE_CMD_IDENTIFY 0xEC

// Drive selection
#define IDE_MASTER      0xA0
#define IDE_SLAVE       0xA0  // CHS mode

bool disk_init(void);
bool disk_read_sector(uint32_t lba, uint8_t *buffer);
bool disk_write_sector(uint32_t lba, const uint8_t *buffer);
bool disk_identify(uint16_t *buffer);

#endif