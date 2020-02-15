#ifndef __FAT_BOOTSEC_H_INCLUDED__
#define __FAT_BOOTSEC_H_INCLUDED__

#include <stdint.h>

typedef struct {
    uint8_t jmpBoot[3];
    char oemName[9];
    uint16_t BPB_bytePerSec;
    uint8_t BPB_secPerClus;
    uint16_t BPB_rsvdSecCnt;
    uint8_t BPB_numFATs;
    uint16_t BPB_rootEntCnt;
    uint16_t BPB_totSec16;
    uint8_t BPB_media;
    uint16_t BPB_fatSz16;
    uint16_t BPB_secPerTrk;
    uint16_t BPB_numHeads;
    uint32_t BPB_hiddSec;
    uint32_t BPB_totSec32;
} FAT_CommBootSec;

typedef struct {
    FAT_CommBootSec comm;
    uint8_t drvNum;
    //uint8_t reserved1;
    uint8_t bootSig;
    uint32_t volId;
    char volLab[12];
    char fsType[9];
} FAT12_BootSec;

typedef FAT12_BootSec FAT16_BootSec;

typedef struct {
    FAT16_BootSec comm;
    uint32_t BPB_fatSz32;
    uint16_t BPB_extFlags;
    //uint16_t BPB_fsVer;
    uint8_t BPB_fsMinorVer;
    uint8_t BPB_fsMajorVer;
    uint32_t BPB_rootClus;
    uint16_t BPB_fsInfo;
    uint16_t BPB_bkBootSec;
    //uint8_t reserved[12];
} FAT32_BootSec;

void FAT_loadCommon(FAT_CommBootSec *fat, uint8_t *buffer);
void FAT16_loadFields(FAT16_BootSec *fat, uint8_t *buffer);
void FAT32_loadFields(FAT32_BootSec *fat, uint8_t *buffer);

void FAT_CommBootSec_display(FAT_CommBootSec *bootSec);
void FAT16_BootSec_display(FAT16_BootSec *bootSec);
void FAT32_BootSec_display(FAT32_BootSec *bootSec);

#endif
