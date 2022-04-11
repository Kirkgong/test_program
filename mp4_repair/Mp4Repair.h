#ifndef MP4_REPAIR_H
#define MP4_REPAIR_H

#include "stdint.h"
#include <fstream>
#include <iostream>
#include "FileMux.h"

// #define BUF_SIZE 3840*1696*3
#define BUF_SIZE 1000000

typedef enum{
    FILE_STATUR_NORMAL,
    FILE_STATUR_ABNORMAL,
    FILE_STATUR_DAMAGE,
    FILE_STATUR_NOTEXIST,
}FILE_STATUS;

typedef enum{
    NAUL_TYPE_NONE = 0,
    NAUL_TYPE_PPS,
    NAUL_TYPE_SLICE,
    NAUL_TYPE_IDR,
}NAUL_TYPE;

class Mp4Repair{
private:
    std::ifstream in_fs; 
    uint8_t buf[BUF_SIZE];
    uint32_t buf_index;

    FileMux file_mux;

    int in_file_len;
    int mdat_index;
    int mdat_len;

public:
    Mp4Repair();
    ~Mp4Repair();

    bool open(char* file);
    FILE_STATUS check(char* file);
    void close();
    FILE_STATUS repair(char* file);

private:
    bool boxParse(FILE_STATUS* status);
    bool boxNeedIterate(const char* box_name);
    bool stringCompare(const char* str1, const char* str2, int len);
    void write( uint8_t* data, uint32_t len, NAUL_TYPE type);
};

#endif