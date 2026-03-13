#include "disk.h"
#include <stdint.h>
#include <stdbool.h>

// Port I/O functions
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void insw(uint16_t port, uint16_t *buffer, uint32_t count) {
    __asm__ volatile ("rep insw" : "+D"(buffer), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, const uint16_t *buffer, uint32_t count) {
    __asm__ volatile ("rep outsw" : "+S"(buffer), "+c"(count) : "d"(port) : "memory");
}

static void ide_wait_ready(void) {
    while (inb(IDE_STATUS) & IDE_STATUS_BSY);
}

static bool ide_wait_drq(void) {
    uint8_t status;
    do {
        status = inb(IDE_STATUS);
        if (status & IDE_STATUS_ERR) return false;
    } while (!(status & IDE_STATUS_DRQ));
    return true;
}

static void ide_select_drive(uint8_t drive, uint32_t lba) {
    outb(IDE_DRIVE_HEAD, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F)));  // LBA mode, master
    outb(IDE_SECTOR_NUM, (uint8_t)lba);
    outb(IDE_CYL_LOW, (uint8_t)(lba >> 8));
    outb(IDE_CYL_HIGH, (uint8_t)(lba >> 16));
    outb(IDE_SECTOR_COUNT, 1);
}

bool disk_init(void) {
    // Select master drive (index 1 in QEMU might be master?)
    outb(IDE_DRIVE_HEAD, IDE_MASTER);
    
    // Wait for drive to be ready
    ide_wait_ready();
    
    // Check if drive is ready
    uint8_t status = inb(IDE_STATUS);
    if (!(status & IDE_STATUS_RDY)) {
        return false;
    }

    return true;
}

bool disk_read_sector(uint32_t lba, uint8_t *buffer) {
    ide_wait_ready();
    ide_select_drive(IDE_MASTER, lba);
    ide_wait_ready();  // Wait after selecting drive
    outb(IDE_COMMAND, IDE_CMD_READ);

    if (!ide_wait_drq()) return false;

    insw(IDE_DATA, (uint16_t *)buffer, 256);
    return true;
}

bool disk_write_sector(uint32_t lba, const uint8_t *buffer) {
    ide_wait_ready();
    ide_select_drive(IDE_MASTER, lba);
    ide_wait_ready();  // Wait after selecting drive
    outb(IDE_COMMAND, IDE_CMD_WRITE);

    if (!ide_wait_drq()) return false;

    outsw(IDE_DATA, (const uint16_t *)buffer, 256);
    return true;
}

bool disk_identify(uint16_t *buffer) {
    ide_wait_ready();
    outb(IDE_DRIVE_HEAD, IDE_MASTER);
    ide_wait_ready();
    outb(IDE_COMMAND, IDE_CMD_IDENTIFY);

    if (!ide_wait_drq()) return false;

    insw(IDE_DATA, buffer, 256);
    return true;
}