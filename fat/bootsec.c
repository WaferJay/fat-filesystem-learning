
#include "bootsec.h"
#include "bytes.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

void FAT_loadCommon(FAT_CommBootSec *fat, uint8_t *buffer)
{
    strncpy(fat->jmpBoot, buffer, 3);
    buffer += 3;

    strncpy(fat->oemName, buffer, 8);
    buffer += 8;

    fat->BPB_bytePerSec = READLE(uint16_t, buffer);
    fat->BPB_secPerClus = READLE(uint8_t, buffer);
    fat->BPB_rsvdSecCnt = READLE(uint16_t, buffer);
    fat->BPB_numFATs = READLE(uint8_t, buffer);
    fat->BPB_rootEntCnt = READLE(uint16_t, buffer);
    fat->BPB_totSec16 = READLE(uint16_t, buffer);
    fat->BPB_media = READLE(uint8_t, buffer);
    fat->BPB_fatSz16 = READLE(uint16_t, buffer);
    fat->BPB_secPerTrk = READLE(uint16_t, buffer);
    fat->BPB_numHeads = READLE(uint16_t, buffer);
    fat->BPB_hiddSec = READLE(uint32_t, buffer);
    fat->BPB_totSec32 = READLE(uint32_t, buffer);
}

void FAT16_loadFields(FAT16_BootSec *fat, uint8_t *buffer)
{
    fat->drvNum = READLE(uint8_t, buffer);
    buffer++;  // sikp reserved1
    fat->bootSig = READLE(uint8_t, buffer);
    fat->volId = READLE(uint32_t, buffer);

    strncpy(fat->volLab, buffer, 11);
    buffer += 11;
    strncpy(fat->fsType, buffer, 8);
    buffer += 8;
}

void FAT32_loadFields(FAT32_BootSec *fat, uint8_t *buffer)
{
    fat->BPB_fatSz32 = READLE(uint32_t, buffer);
    fat->BPB_extFlags = READLE(uint16_t, buffer);
    fat->BPB_fsMinorVer = READLE(uint8_t, buffer);
    fat->BPB_fsMajorVer = READLE(uint8_t, buffer);
    fat->BPB_rootClus = READLE(uint32_t, buffer);
    fat->BPB_fsInfo = READLE(uint16_t, buffer);
    fat->BPB_bkBootSec = READLE(uint16_t, buffer);
    buffer += 12;  // skip reserved
    FAT16_loadFields(&fat->comm, buffer);
}

void FAT_CommBootSec_display(FAT_CommBootSec *bootSec)
{
    printf("Jump instruction: 0x%X 0x%X 0x%X\n", bootSec->jmpBoot[0],
            bootSec->jmpBoot[1], bootSec->jmpBoot[2]);
    printf("OEM Name: %s\n", bootSec->oemName);
    printf("Bytes Per Sector: %u\n", bootSec->BPB_bytePerSec);
    printf("Sectors Per Cluster: %u\n", bootSec->BPB_secPerClus);
    printf("Resvered Sector Count: %u\n", bootSec->BPB_rsvdSecCnt);
    printf("Number of FATs: %u\n", bootSec->BPB_numFATs);
    printf("Root Entries Count: %u\n", bootSec->BPB_rootEntCnt);
    printf("Total Sectors (old 16-bit): %u\n", bootSec->BPB_totSec16);
    printf("Media: 0x%X\n", bootSec->BPB_media);
    printf("Sectors occupied by a FAT (16-bit): %u\n", bootSec->BPB_fatSz16);
    printf("Sectors per Track: %u\n", bootSec->BPB_secPerTrk);
    printf("Number of heads: %u\n", bootSec->BPB_numHeads);
    printf("Hidden Physical Sectors: %u\n", bootSec->BPB_hiddSec);
    printf("Total Sectors (new 32-bit): %u\n", bootSec->BPB_totSec32);
}

void FAT16_BootSec_display(FAT16_BootSec *bootSec)
{
    printf("Drive Number: 0x%X\n", bootSec->drvNum);
    printf("Extended boot signature: 0x%X\n", bootSec->bootSig);
    printf("Volume ID: 0x%X\n", bootSec->volId);
    printf("Volume Label: %s\n", bootSec->volLab);
    printf("Filesystem type: %s\n", bootSec->fsType);
}

void FAT32_BootSec_display(FAT32_BootSec *bootSec)
{
    printf("Sectors occupied by a FAT (32-bit): %u\n", bootSec->BPB_fatSz32);
    printf("Flags: 0b%o\n", bootSec->BPB_extFlags);
    printf("Filesystem Version: %u.%u\n", bootSec->BPB_fsMajorVer,
            bootSec->BPB_fsMinorVer);
    printf("Root Directory Clusters: %u\n", bootSec->BPB_rootClus);
    printf("FSInfo Offset: %u\n", bootSec->BPB_fsInfo);
    printf("Backup Boot Sector Offset: %u\n", bootSec->BPB_bkBootSec);
    FAT16_BootSec_display((FAT16_BootSec *) bootSec);
}
