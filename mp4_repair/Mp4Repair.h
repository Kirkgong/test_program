#ifndef MP4_REPAIR_H
#define MP4_REPAIR_H

#include "stdint.h"
#include <fstream>
#include <iostream>

// #define BUF_SIZE 3840*1696*3
#define BUF_SIZE 1000000

typedef enum{
    FILE_STATUR_NORMAL,
    FILE_STATUR_ABNORMAL,
    FILE_STATUR_DAMAGE,
    FILE_STATUR_NOTEXIST,
}FILE_STATUS;

class Mp4Repair{
private:
    std::ifstream in_fs; 
    uint8_t buf[BUF_SIZE];
    uint32_t buf_index;

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
};

#endif