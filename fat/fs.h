#ifndef __FAT_FS_H_INCLUDED__
#define __FAT_FS_H_INCLUDED__

#include "bootsec.h"

#include <stdint.h>

typedef unsigned int uint;

typedef enum {
    FAT_STATUS_OK,
    FAT_STATUS_INVAL,
    FAT_STATUS_EOF,
    FAT_STATUS_ACCESS_DENIED,
    FAT_STATUS_NO_MEM,
    FAT_STATUS_UNSUPPORTED
} FAT_Status;

typedef enum {
    FAT_SUBTYPE_FAT12,
    FAT_SUBTYPE_FAT16,
    FAT_SUBTYPE_FAT32
} FAT_subtype;

typedef struct {
    char *devPath;
    int devFd;
    int fdFlags;
    FAT_subtype subtype;
    FAT_CommBootSec *bootSec;

    uint32_t fatSize;
    uint32_t totalSectors;
    uint16_t fatStart;
    uint fatSectors;
    uint rootDirStart;
    uint rootDirSectors;
    uint dataStart;
    uint dataSectors;
    uint totalClusters;
} FAT_fs;

void FAT_close(FAT_fs *fs);
FAT_Status FAT_open(FAT_fs *fs, char *path);

void FAT_display(FAT_fs *fs);

#endif
