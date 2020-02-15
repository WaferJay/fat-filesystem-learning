
#include "fs.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void FAT_close(FAT_fs *fs)
{
    close(fs->devFd);
    fs->devFd = -1;

    free(fs->bootSec);
    fs->bootSec = NULL;
}

FAT_Status FAT_open(FAT_fs *fs, char *path)
{
    fs->devPath = path;
    fs->devFd = open(path, O_RDONLY);
    if (fs->devFd < 0) {
        return FAT_STATUS_ACCESS_DENIED;
    }
    fs->fdFlags = O_RDONLY;

    uint8_t bootSecBuf[512];
    ssize_t readCount = read(fs->devFd, bootSecBuf, 512);
    if (readCount != 512) {
        FAT_close(fs);
        return FAT_STATUS_EOF;
    }

    FAT_CommBootSec *p_fat = (FAT_CommBootSec *) malloc(sizeof(FAT16_BootSec));
    FAT_loadCommon(p_fat, bootSecBuf);
    fs->fatStart = p_fat->BPB_rsvdSecCnt;
    fs->totalSectors = p_fat->BPB_totSec16 ? p_fat->BPB_totSec16
            : p_fat->BPB_totSec32;

    if (p_fat->BPB_fatSz16) {
        // FAT12/16
        fs->fatSize = p_fat->BPB_fatSz16;
        FAT16_loadFields((FAT16_BootSec *) p_fat, bootSecBuf+36);
        fs->bootSec = p_fat;
    } else {
        // FAT32
        FAT32_BootSec *p_fat32BootSec;
        p_fat32BootSec = (FAT32_BootSec *) realloc(p_fat, sizeof(FAT32_BootSec));
        if (p_fat32BootSec <= 0) {
            FAT_close(fs);
            return FAT_STATUS_NO_MEM;
        }
        FAT32_loadFields(p_fat32BootSec, bootSecBuf+36);
        fs->fatSize = p_fat32BootSec->BPB_fatSz32;
        fs->bootSec = (FAT_CommBootSec *) p_fat32BootSec;
    }
    p_fat = NULL;  // dont use this variable anymore

    fs->fatSectors = fs->fatSize * fs->bootSec->BPB_numFATs;
    fs->rootDirStart = fs->fatStart + fs->fatSectors;

    const uint rootEntCnt = fs->bootSec->BPB_rootEntCnt;
    const uint bytesPerSec = fs->bootSec->BPB_bytePerSec;
    fs->rootDirSectors = (32 * rootEntCnt + bytesPerSec - 1) / bytesPerSec;

    fs->dataStart = fs->rootDirStart + fs->rootDirSectors;
    fs->dataSectors = fs->totalSectors - fs->dataStart;

    uint clusters = fs->dataSectors / fs->bootSec->BPB_secPerClus;
    fs->totalClusters = clusters;
    if (clusters <= 4085) {
        fs->subtype = FAT_SUBTYPE_FAT12;
    } else if (clusters <= 65525) {
        fs->subtype = FAT_SUBTYPE_FAT16;
    } else {
        fs->subtype = FAT_SUBTYPE_FAT32;
    }

    return FAT_STATUS_OK;
}

void FAT_display(FAT_fs *fs)
{
    printf("* Sectors occupied by a FAT: %u\n", fs->fatSize);
    printf("* Total Sectors: %u\n", fs->totalSectors);
    printf("* FAT Start: %u\n", fs->fatStart);
    printf("* FAT Sectors: %u\n", fs->fatSectors);
    printf("* Root Directory Start: %u\n", fs->rootDirStart);
    printf("* Root Directory Sectors: %u\n", fs->rootDirSectors);
    printf("* Data Start: %u\n", fs->dataStart);
    printf("* Data Sectors: %u\n", fs->dataSectors);
    printf("* Total Clusters: %u\n", fs->totalClusters);
    putchar('\n');

    puts("===== Common Fields =====");
    FAT_CommBootSec_display(fs->bootSec);
    putchar('\n');

    switch (fs->subtype) {
    case FAT_SUBTYPE_FAT12:
    case FAT_SUBTYPE_FAT16:
        puts("===== FAT12/16 Fields =====");
        FAT16_BootSec_display((FAT16_BootSec *) fs->bootSec);
        break;
    case FAT_SUBTYPE_FAT32:
        puts("====== FAT32 Fields ======");
        FAT32_BootSec_display((FAT32_BootSec *) fs->bootSec);
        break;
    }
    putchar('\n');
}

FAT_Status FAT_readSectors(FAT_fs *fs, size_t clusIdx, size_t secStart,
        size_t secCount, void *mem, size_t memsize)
{
    if (clusIdx >= fs->totalClusters) {
        return FAT_STATUS_EOF;
    }

    const int bytesPerSec = fs->bootSec->BPB_bytePerSec;
    const int secPerClus = fs->bootSec->BPB_secPerClus;
    if (secStart >= secPerClus) {
        return FAT_STATUS_INVAL;
    }

    if (!secCount) {
        secCount = secPerClus - secStart;
    } else if (secCount > secPerClus - secStart) {
        return FAT_STATUS_INVAL;
    }

    size_t readCount = secCount * bytesPerSec;
    readCount = readCount > memsize ? memsize : readCount;

    off_t start = fs->dataStart * bytesPerSec;
    off_t actualOffset = lseek(fs->devFd, start, SEEK_SET);
    if (start != actualOffset) {
        return FAT_STATUS_EOF;
    }

    off_t offsetBytes = (clusIdx * secPerClus + secStart) * bytesPerSec;
    actualOffset = lseek(fs->devFd, offsetBytes, SEEK_CUR) - actualOffset;
    if (actualOffset != offsetBytes) {
        return FAT_STATUS_EOF;
    }

    ssize_t actualRead = read(fs->devFd, mem, readCount);
    if (actualRead != readCount) {
        return FAT_STATUS_EOF;
    }

    return FAT_STATUS_OK;
}
