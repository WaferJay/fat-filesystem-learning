#ifndef __FAT_ENTRY_H_INCLUDED__
#define __FAT_ENTRY_H_INCLUDED__

#include "fs.h"

#include <stdint.h>
#include <time.h>

#define ENTRY_ATTR_READ_ONLY ((uint8_t) 0x01)
#define ENTRY_ATTR_HIDDEN ((uint8_t) 0x02)
#define ENTRY_ATTR_SYSTEM ((uint8_t) 0x04)
#define ENTRY_ATTR_VOLUMNE_ID ((uint8_t) 0x08)
#define ENTRY_ATTR_DIRECTORY ((uint8_t) 0x10)
#define ENTRY_ATTR_ARCHIVE ((uint8_t) 0x20)
#define ENTRY_ATTR_LONG_FILE_NAME ((uint8_t) 0x0f)

typedef struct {
    char *name;
    uint8_t value;
} FAT_EntryAttr;

#define __ENTRY_ATTR(attr) {#attr, attr}

#define COUNT_ENTRY_ATTR 7
const static FAT_EntryAttr __allEntryAttrs[COUNT_ENTRY_ATTR] = {
    __ENTRY_ATTR(ENTRY_ATTR_READ_ONLY),
    __ENTRY_ATTR(ENTRY_ATTR_HIDDEN),
    __ENTRY_ATTR(ENTRY_ATTR_SYSTEM),
    __ENTRY_ATTR(ENTRY_ATTR_VOLUMNE_ID),
    __ENTRY_ATTR(ENTRY_ATTR_DIRECTORY),
    __ENTRY_ATTR(ENTRY_ATTR_ARCHIVE),
    __ENTRY_ATTR(ENTRY_ATTR_LONG_FILE_NAME)
};
#define ENTRY_ATTRS (__allEntryAttrs)

typedef enum {
    FAT_TIME_TYPE_WRITE,
    FAT_TIME_TYPE_CREATE,
    FAT_TIME_TYPE_ACCESS,
} FAT_TimeType;

typedef struct {
    uint8_t attr;
    uint8_t name[12];  // short file name
    uint8_t ntRes;  // case
    uint8_t crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t fstClusHi;
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t fstClusLo;
    uint32_t fileSize;
} FAT_DirEntry;

typedef struct {
    uint8_t attr;
    uint8_t ord;
    uint8_t name1[10];
    uint8_t type;
    uint8_t chksum;
    uint8_t name2[12];
    uint16_t fstClusLo;
    uint8_t name3[4];
} FAT_LfnEntry;

typedef union {
    uint8_t attr;
    FAT_DirEntry dir;
    FAT_LfnEntry lfn;
} FAT_Entry;

void FAT_loadEntry(FAT_Entry *entry, uint8_t buf[32]);
FAT_Status FAT_DirEntry_time(FAT_DirEntry *dir, FAT_TimeType timetype,
        struct tm *t);
FAT_Status FAT_DirEntry_ts(FAT_DirEntry *dir, FAT_TimeType type, time_t *ts);

typedef int (*entry_filter)(FAT_Entry *);
FAT_Status FAT_walkEntry(FAT_fs *fs, entry_filter filter);

uint8_t FAT_DirEntry_checksum(FAT_DirEntry *);

int FAT_LfnEntry_concat(FAT_LfnEntry *lfn, char *buf,
        int start, int end);

void FAT_LfnEntry_display(FAT_LfnEntry *lfn);
void FAT_DirEntry_display(FAT_DirEntry *dir);
void FAT_Entry_display(FAT_Entry *entry);

#endif
