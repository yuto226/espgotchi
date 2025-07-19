/*
 * FatFS compatibility layer for ESP32
 * Maps FatFS functions to standard C file operations
 */
#ifndef _FATFS_COMPAT_H_
#define _FATFS_COMPAT_H_

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct {
    FILE* fp;
} FIL;

typedef unsigned int UINT;

#define FR_OK           0
#define FR_DISK_ERR     1
#define FR_INT_ERR      2
#define FR_NOT_READY    3
#define FR_NO_FILE      4
#define FR_NO_PATH      5
#define FR_INVALID_NAME 6
#define FR_DENIED       7
#define FR_EXIST        8
#define FR_INVALID_OBJECT 9
#define FR_WRITE_PROTECTED 10
#define FR_INVALID_DRIVE 11
#define FR_NOT_ENABLED  12
#define FR_NO_FILESYSTEM 13
#define FR_MKFS_ABORTED 14
#define FR_TIMEOUT      15
#define FR_LOCKED       16
#define FR_NOT_ENOUGH_CORE 17
#define FR_TOO_MANY_OPEN_FILES 18

#define FA_READ         0x01
#define FA_WRITE        0x02
#define FA_OPEN_EXISTING 0x00
#define FA_CREATE_NEW   0x04
#define FA_CREATE_ALWAYS 0x08
#define FA_OPEN_ALWAYS  0x10

static inline int f_open(FIL* fil, const char* path, unsigned char mode) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "/spiffs/%s", path);
    
    const char* fmode;
    if (mode & FA_CREATE_ALWAYS) {
        fmode = (mode & FA_WRITE) ? "w" : "r";
    } else if (mode & FA_OPEN_EXISTING) {
        fmode = (mode & FA_WRITE) ? "r+" : "r";
    } else {
        fmode = "r";
    }
    
    fil->fp = fopen(full_path, fmode);
    return (fil->fp == NULL) ? FR_NO_FILE : FR_OK;
}

static inline int f_close(FIL* fil) {
    if (fil && fil->fp && fclose(fil->fp) == 0) {
        fil->fp = NULL;
        return FR_OK;
    }
    return FR_DISK_ERR;
}

static inline int f_read(FIL* fil, void* buff, UINT btr, UINT* br) {
    if (!fil || !fil->fp || !buff || !br) return FR_INVALID_OBJECT;
    *br = fread(buff, 1, btr, fil->fp);
    return (*br == btr || feof(fil->fp)) ? FR_OK : FR_DISK_ERR;
}

static inline int f_write(FIL* fil, const void* buff, UINT btw, UINT* bw) {
    if (!fil || !fil->fp || !buff || !bw) return FR_INVALID_OBJECT;
    *bw = fwrite(buff, 1, btw, fil->fp);
    return (*bw == btw) ? FR_OK : FR_DISK_ERR;
}

static inline int f_stat(const char* path, void* fno) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "/spiffs/%s", path);
    
    struct stat st;
    return (stat(full_path, &st) == 0) ? FR_OK : FR_NO_FILE;
}

#endif /* _FATFS_COMPAT_H_ */