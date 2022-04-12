#ifndef MP4_REPAIR_H
#define MP4_REPAIR_H

#include "stdint.h"
#include <fstream>
#include <iostream>
#include "FileMux.h"

// #define BUF_SIZE 3840*1696*3
#define BUF_SIZE 1000000

typedef enum{
    FILE_STATUR_NORMAL,                     // File normal.Don't need repair.
    FILE_STATUR_ABNORMAL,                   // File lack of moov.
    FILE_STATUR_DAMAGE,                     // File damage.Can't repair.
    FILE_STATUR_REPAIR_FAIL,                // File repair fail.
    FILE_STATUR_REPAIR_SUCCESS,             // File repair success.
    FILE_STATUR_UNDEFINE,
}FILE_STATUS;

typedef enum{
    NAUL_TYPE_NONE = 0,
    NAUL_TYPE_PPS,
    NAUL_TYPE_SLICE,
    NAUL_TYPE_IDR,
}NAUL_TYPE;

class Mp4Repair{
private:
    uint8_t* buf;
    uint32_t mdat_box_index;
    uint32_t mdat_box_len;

public:
    Mp4Repair();
    ~Mp4Repair();

    FILE_STATUS process(char* file);

private:
    bool boxParse(std::ifstream* fs, FILE_STATUS* status, uint32_t file_len);
    bool boxNeedIterate(const char* box_name);
    bool stringCompare(const char* str1, const char* str2, int len);
    void write(FileMux* file_mux, uint8_t* data, uint32_t len, NAUL_TYPE type);
    uint32_t getFileLen(std::ifstream* fs, uint32_t seek);
    std::ifstream* openFile(char* file);
    FILE_STATUS checkFile(char* file);
    void closeFile(std::ifstream* fs);
    FILE_STATUS repair(char* file);
    void setMdatBoxIndex(uint32_t index);
    void setMdatBoxLen(uint32_t len);
    uint32_t getMdatBoxIndex(void);
    uint32_t getMdatBoxLen(void);
};

#endif