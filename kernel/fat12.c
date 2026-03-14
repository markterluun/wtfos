#include "fat12.h"
#include "disk.h"
#include "string.h"
#include "endian.h"
#include "io.h"

static struct fat12_boot_sector boot_sector;
static uint8_t *fat_buffer = NULL;
static uint8_t *root_buffer = NULL;
static uint32_t fat_size;
static uint32_t root_size;
static uint32_t data_start_sector;

static uint16_t fat12_get_fat_entry(uint16_t cluster) {
    uint32_t offset = cluster * 3 / 2;
    uint16_t val = *(uint16_t*)&fat_buffer[offset];
    val = le16toh(val);
    if (cluster & 1) {
        return val >> 4;
    } else {
        return val & 0xFFF;
    }
}

static void fat12_set_fat_entry(uint16_t cluster, uint16_t value) {
    uint32_t offset = cluster * 3 / 2;
    uint16_t *entry = (uint16_t*)&fat_buffer[offset];
    uint16_t val = le16toh(*entry);
    if (cluster & 1) {
        val = (val & 0x000F) | (value << 4);
    } else {
        val = (val & 0xF000) | (value & 0xFFF);
    }
    *entry = htole16(val);
}

static uint32_t cluster_to_sector(uint16_t cluster) {
    return data_start_sector + (cluster - 2) * boot_sector.sectors_per_cluster;
}

int fat12_init(void) {
    uint8_t buffer[512];
    if (!disk_read_sector(0, buffer)) {
        kio_println("Failed to read boot sector");
        return -1;
    }

    memcpy(&boot_sector, buffer, sizeof(boot_sector));
    boot_sector.signature = le16toh(*(uint16_t*)&buffer[510]);
    if (boot_sector.signature != FAT12_SIGNATURE) {
        kio_println("Invalid FAT12 signature");
        return -1;
    }

    // Calculate sizes
    fat_size = boot_sector.sectors_per_fat * boot_sector.bytes_per_sector;
    root_size = boot_sector.root_dir_entries * 32; // 32 bytes per entry
    uint32_t root_sectors = (root_size + boot_sector.bytes_per_sector - 1) / boot_sector.bytes_per_sector;
    data_start_sector = boot_sector.reserved_sectors + boot_sector.num_fats * boot_sector.sectors_per_fat + root_sectors;

    // Allocate buffers
    fat_buffer = kmalloc(fat_size);
    root_buffer = kmalloc(root_size);
    if (!fat_buffer || !root_buffer) {
        kio_println("Failed to allocate FAT12 buffers");
        return -1;
    }

    // Load FAT
    for (uint32_t i = 0; i < boot_sector.sectors_per_fat; i++) {
        if (!disk_read_sector(boot_sector.reserved_sectors + i, &fat_buffer[i * 512])) {
            kio_println("Failed to read FAT");
            return -1;
        }
    }

    // Load root directory
    uint32_t root_start = boot_sector.reserved_sectors + boot_sector.num_fats * boot_sector.sectors_per_fat;
    for (uint32_t i = 0; i < root_sectors; i++) {
        if (!disk_read_sector(root_start + i, &root_buffer[i * 512])) {
            kio_println("Failed to read root directory");
            return -1;
        }
    }

    kio_println("FAT12 initialized");
    return 0;
}

static struct fat12_dir_entry *find_file(const char *filename) {
    char fname[12];
    memset(fname, ' ', 11);
    fname[11] = '\0';
    strcpy(fname, filename);
    // Convert to uppercase
    for (int i = 0; fname[i]; i++) {
        if (fname[i] >= 'a' && fname[i] <= 'z') {
            fname[i] -= 32;
        }
    }

    for (uint32_t i = 0; i < boot_sector.root_dir_entries; i++) {
        struct fat12_dir_entry *entry = (struct fat12_dir_entry*)&root_buffer[i * 32];
        if (entry->name[0] == 0x00 || entry->name[0] == 0xE5) continue;
        if (memcmp(entry->name, fname, 11) == 0) {
            return entry;
        }
    }
    return NULL;
}

int fat12_read_file(const char *filename, uint8_t *buffer, uint32_t *size) {
    struct fat12_dir_entry *entry = find_file(filename);
    if (!entry) {
        kio_println("File not found");
        return -1;
    }

    uint16_t cluster = le16toh(entry->first_cluster_low);
    uint32_t file_size = le32toh(entry->size);
    *size = file_size;
    uint32_t bytes_read = 0;

    while (cluster < 0xFF8 && bytes_read < file_size) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint8_t s = 0; s < boot_sector.sectors_per_cluster && bytes_read < file_size; s++) {
            uint32_t to_read = boot_sector.bytes_per_sector;
            if (bytes_read + to_read > file_size) to_read = file_size - bytes_read;
            if (!disk_read_sector(sector + s, &buffer[bytes_read])) {
                kio_println("Failed to read file sector");
                return -1;
            }
            bytes_read += to_read;
        }
        cluster = fat12_get_fat_entry(cluster);
    }

    return 0;
}

int fat12_write_file(const char *filename, const uint8_t *data, uint32_t size) {
    // Find free directory entry
    struct fat12_dir_entry *free_entry = NULL;
    for (uint32_t i = 0; i < boot_sector.root_dir_entries; i++) {
        struct fat12_dir_entry *entry = (struct fat12_dir_entry*)&root_buffer[i * 32];
        if (entry->name[0] == 0x00 || entry->name[0] == 0xE5) {
            free_entry = entry;
            break;
        }
    }
    if (!free_entry) {
        kio_println("No free directory entry");
        return -1;
    }

    // Allocate clusters
    uint16_t first_cluster = 0;
    uint16_t prev_cluster = 0;
    uint32_t clusters_needed = (size + boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector - 1) / (boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector);
    for (uint32_t c = 0; c < clusters_needed; c++) {
        uint16_t cluster = 3; // Start from 3
        for (; cluster < 0xFF8; cluster++) {
            if (fat12_get_fat_entry(cluster) == 0) break;
        }
        if (cluster >= 0xFF8) {
            kio_println("No free clusters");
            return -1;
        }
        if (c == 0) first_cluster = cluster;
        if (prev_cluster) fat12_set_fat_entry(prev_cluster, cluster);
        prev_cluster = cluster;
    }
    if (prev_cluster) fat12_set_fat_entry(prev_cluster, 0xFFF);

    // Write data
    uint16_t cluster = first_cluster;
    uint32_t bytes_written = 0;
    while (cluster < 0xFF8 && bytes_written < size) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint8_t s = 0; s < boot_sector.sectors_per_cluster && bytes_written < size; s++) {
            uint32_t to_write = boot_sector.bytes_per_sector;
            if (bytes_written + to_write > size) to_write = size - bytes_written;
            if (!disk_write_sector(sector + s, (uint8_t*)&data[bytes_written])) {
                kio_println("Failed to write file sector");
                return -1;
            }
            bytes_written += to_write;
        }
        cluster = fat12_get_fat_entry(cluster);
    }

    // Update directory entry
    char fname[12];
    memset(fname, ' ', 11);
    fname[11] = '\0';
    strcpy(fname, filename);
    for (int i = 0; fname[i]; i++) {
        if (fname[i] >= 'a' && fname[i] <= 'z') {
            fname[i] -= 32;
        }
    }
    memcpy(free_entry->name, fname, 11);
    free_entry->attributes = ATTR_ARCHIVE;
    free_entry->first_cluster_low = htole16(first_cluster);
    free_entry->size = htole32(size);

    // Write back FAT and root
    for (uint32_t i = 0; i < boot_sector.sectors_per_fat; i++) {
        disk_write_sector(boot_sector.reserved_sectors + i, &fat_buffer[i * 512]);
    }
    uint32_t root_start = boot_sector.reserved_sectors + boot_sector.num_fats * boot_sector.sectors_per_fat;
    uint32_t root_sectors = (root_size + boot_sector.bytes_per_sector - 1) / boot_sector.bytes_per_sector;
    for (uint32_t i = 0; i < root_sectors; i++) {
        disk_write_sector(root_start + i, &root_buffer[i * 512]);
    }

    return 0;
}

int fat12_list_dir(void) {
    for (uint32_t i = 0; i < boot_sector.root_dir_entries; i++) {
        struct fat12_dir_entry *entry = (struct fat12_dir_entry*)&root_buffer[i * 32];
        if (entry->name[0] == 0x00) break;
        if (entry->name[0] == 0xE5) continue;
        if (entry->attributes & (ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)) continue;
        char name[12];
        memcpy(name, entry->name, 11);
        name[11] = '\0';
        // Trim spaces
        for (int j = 10; j >= 0; j--) {
            if (name[j] == ' ') name[j] = '\0';
            else break;
        }
        kio_print(name);
        if (entry->attributes & ATTR_DIRECTORY) {
            kio_print(" <DIR>");
        } else {
            kio_print(" ");
            // Print size
            char size_str[16];
            uint32_t size = le32toh(entry->size);
            int len = 0;
            if (size == 0) size_str[len++] = '0';
            else {
                while (size) {
                    size_str[len++] = '0' + (size % 10);
                    size /= 10;
                }
                for (int j = 0; j < len / 2; j++) {
                    char tmp = size_str[j];
                    size_str[j] = size_str[len - 1 - j];
                    size_str[len - 1 - j] = tmp;
                }
            }
            size_str[len] = '\0';
            kio_print(size_str);
        }
        kio_println("");
    }
    return 0;
}