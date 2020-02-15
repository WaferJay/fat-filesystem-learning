
#include "bytes.h"
#include "charset.h"
#include "entry.h"
#include "fs.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

static void loadLongFileNameEntry(FAT_LfnEntry *lfn, uint8_t buf[32])
{
    lfn->ord = READLE(uint8_t, buf);
    memcpy(lfn->name1, buf, 10);
    buf += 11;  // include attr field

    lfn->type = READLE(uint8_t, buf);
    lfn->chksum = READLE(uint8_t, buf);

    memcpy(lfn->name2, buf, 12);
    buf += 12;
    lfn->fstClusLo = READLE(uint16_t, buf);

    memcpy(lfn->name3, buf, 4);
    buf += 4;
}

static void loadDirectoryEntry(FAT_DirEntry *dir, uint8_t buf[32])
{
    strncpy(dir->name, buf, 11);
    buf += 12;  // include attr field
    dir->ntRes = READLE(uint8_t, buf);
    dir->crtTimeTenth = READLE(uint8_t, buf);
    dir->crtTime = READLE(uint16_t, buf);
    dir->crtDate = READLE(uint16_t, buf);
    dir->lstAccDate = READLE(uint16_t, buf);
    dir->fstClusHi = READLE(uint16_t, buf);
    dir->wrtTime = READLE(uint16_t, buf);
    dir->wrtDate = READLE(uint16_t, buf);
    dir->fstClusLo = READLE(uint16_t, buf);
    dir->fileSize = READLE(uint32_t, buf);
}

void FAT_loadEntry(FAT_Entry *entry, uint8_t buf[32])
{
    uint8_t attr = buf[11];
    entry->attr = attr;
    if (CHECK_MASK(attr, ENTRY_ATTR_LONG_FILE_NAME)) {
        loadLongFileNameEntry(&entry->lfn, buf);
    } else {
        loadDirectoryEntry(&entry->dir, buf);
    }
}

static int parseEntryDate(uint16_t date, struct tm *t)
{
    int dayOfMonth = date & 0x001f;
    int monthOfYear = (date & 0x01e0) >> 5;
    if (monthOfYear > 12 || monthOfYear < 1) {
        return -1;
    }
    int year = ((date & 0xfe00) >> 9) + 1980;
    t->tm_mday = dayOfMonth;
    t->tm_mon = monthOfYear - 1;
    t->tm_year = year - 1900;
    return 0;
}

static int parseEntryTime(uint16_t time, struct tm *t)
{
    int second = (time & 0x001f) * 2;
    if (second > 58) {
        return -1;
    }
    int minute = (time & 0x07e0) >> 5;
    if (minute > 59) {
        return -2;
    }
    int hour = (time & 0xf800) >> 11;
    if (hour > 23) {
        return -3;
    }
    t->tm_sec = second;
    t->tm_min = minute;
    t->tm_hour = hour;
    return 0;
}

FAT_Status FAT_DirEntry_time(FAT_DirEntry *dir, FAT_TimeType timetype, struct tm *t)
{
    switch (timetype) {
    case FAT_TIME_TYPE_WRITE:
        if (parseEntryDate(dir->wrtDate, t)
                || parseEntryTime(dir->wrtTime, t)) {

            return FAT_STATUS_INVAL;
        }

        break;
    case FAT_TIME_TYPE_CREATE:
        if (!dir->crtDate) {
            return FAT_STATUS_UNSUPPORTED;
        }

        if (parseEntryDate(dir->crtDate, t)
                || parseEntryTime(dir->crtTime, t)) {

            return FAT_STATUS_INVAL;
        }

        if (dir->crtTimeTenth) {
            t->tm_sec += dir->crtTimeTenth / 200;
            // XXX: the remaining digits are discarded
        }
        break;
    case FAT_TIME_TYPE_ACCESS:
        if (!dir->lstAccDate) {
            return FAT_STATUS_UNSUPPORTED;
        }

        if (parseEntryDate(dir->lstAccDate, t)) {
            return FAT_STATUS_INVAL;
        }
        parseEntryTime(0, t);
        break;
    }

    t->tm_isdst = -1;
    return FAT_STATUS_OK;
}

FAT_Status FAT_DirEntry_ts(FAT_DirEntry *dir, FAT_TimeType type, time_t *ts)
{
    struct tm tm;
    FAT_Status status = FAT_DirEntry_time(dir, type, &tm);
    if (status == FAT_STATUS_OK) {
        *ts = mktime(&tm);
    }
    return status;
}

FAT_Status FAT_walkEntry(FAT_fs *fs, entry_filter filter)
{
    char buf[32];
    off_t startOffset, actualOffset;

    switch (fs->subtype) {
    case FAT_SUBTYPE_FAT12:
    case FAT_SUBTYPE_FAT16:
        startOffset = fs->rootDirStart * fs->bootSec->BPB_bytePerSec;
        actualOffset = lseek(fs->devFd, startOffset, SEEK_SET);
        if (startOffset != actualOffset) {
            return FAT_STATUS_EOF;
        }

        for (int i=0; i<fs->bootSec->BPB_rootEntCnt; i++) {
            ssize_t readCnt= read(fs->devFd, buf, 32);
            if (readCnt != 32) {
                return FAT_STATUS_EOF;
            }

            if (!buf[0]) break;

            FAT_Entry entry;
            FAT_loadEntry(&entry, buf);
            int brk = filter(&entry);
            if (brk) break;
        }
        break;
    case FAT_SUBTYPE_FAT32:
        // TODO:
        break;
    }
    return FAT_STATUS_OK;
}

uint8_t FAT_DirEntry_checksum(FAT_DirEntry *dir)
{
    uint8_t *name = dir->name;
    uint8_t checksum = 0;
    for (int i=0; i<11; i++) {
        checksum = (checksum >> 1 | checksum << 7) + name[i];
    }

    return checksum;
}

int FAT_LfnEntry_concat(FAT_LfnEntry *lfn, char *buf,
        int start, int end)
{
}

void FAT_DirEntry_display(FAT_DirEntry *dir)
{
    uint8_t *name = dir->name;
    if (*name == 0xe5 || *name == 0x05) {
        printf("First Byte (free): 0x%X\n", *name++);
    }
    printf("Short File Name: %s$\n", name);

    printf("Attribute: 0x%X\n", dir->attr);

    printf("NT Res: 0x%X\n", dir->ntRes);
    printf("Creation Sub-second: %u\n", dir->crtTimeTenth);
    printf("Creation time: %u\n", dir->crtTime);
    printf("Creation date: %u\n", dir->crtDate);
    printf("Last Access Date: %u\n", dir->lstAccDate);
    printf("Write Time: %u\n", dir->wrtTime);
    printf("Write Date: %u\n", dir->wrtDate);

    struct tm tm, *ptr_tm = &tm;
    time_t ts;
    time_t *ptr_ts = &ts;
    FAT_Status stat = FAT_DirEntry_ts(dir, FAT_TIME_TYPE_WRITE, ptr_ts);
    if (stat == FAT_STATUS_OK) {
        printf("Write : %ld %s", ts, ctime(&ts));
    }

    stat = FAT_DirEntry_ts(dir, FAT_TIME_TYPE_CREATE, ptr_ts);
    if (stat == FAT_STATUS_OK) {
        printf("Create: %ld %s", ts, ctime(&ts));
    }

    stat = FAT_DirEntry_ts(dir, FAT_TIME_TYPE_ACCESS, ptr_ts);
    if (stat == FAT_STATUS_OK) {
        printf("Access: %ld %s", ts, ctime(&ts));
    }

    printf("First Cluster Higher: 0x%X\n", dir->fstClusHi);
    printf("First Cluster Lower: 0x%X\n", dir->fstClusLo);
    printf("First Cluster: 0x%X\n", dir->fstClusHi << 16 | dir->fstClusLo);
}

void FAT_LfnEntry_display(FAT_LfnEntry *lfn)
{
    printf("Order: 0x%X\n", lfn->ord);
    printf("Checksum: 0x%X\n", lfn->chksum);
    // considering BOM and NUL, the length plus 2
    wchar_t name1[7] = {0}, name2[8] = {0}, name3[4] = {0};

    utf16le_decode(lfn->name1, 10, name1, 7);
    if (lfn->name2[0] && lfn->name2[0] != 0xff) {
        utf16le_decode(lfn->name2, 12, name2, 8);
    }
    if (lfn->name3[0] && lfn->name3[0] != 0xff) {
        utf16le_decode(lfn->name3,  4, name3, 4);
    }

    printf("Name #1: %ls$\n", name1);
    printf("Name #2: %ls$\n", name2);
    printf("Name #3: %ls$\n", name3);

    printf("Attribute: 0x%X\n", lfn->attr);
    printf("Type: 0x%X\n", lfn->type);
    printf("First Cluster Lower: 0x%X\n", lfn->fstClusLo);
}

void FAT_Entry_display(FAT_Entry *entry)
{
    if (CHECK_MASK(entry->attr, ENTRY_ATTR_LONG_FILE_NAME)) {
        puts("==== Long File Name Entry ====");
        FAT_LfnEntry_display(&entry->lfn);
    } else {
        puts("====== Directory Entry ======");
        FAT_DirEntry_display(&entry->dir);
    }
    puts("\nAttributes:");
    if (entry->attr == ENTRY_ATTR_LONG_FILE_NAME) {
        puts("- ENTRY_LONG_FILE_NAME");
    } else {
        for (int i=0; i<COUNT_ENTRY_ATTR; i++) {
            char *name = ENTRY_ATTRS[i].name;
            uint8_t attr = ENTRY_ATTRS[i].value;
            if (CHECK_MASK(entry->attr, attr)) {
                printf("- %s\n", name);
            }
        }
    }
    putchar('\n');
}
