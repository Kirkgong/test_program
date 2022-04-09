#include "stdint.h"
#include <fstream>
#include <iostream>

#define BUF_SIZE 1024


typedef enum{
    FILE_STATUR_NORMAL,
    FILE_STATUR_ABNORMAL,
    FILE_STATUR_DAMAGE,
    FILE_STATUR_NOTEXIST,
}FILE_STATUS;

class Mp4Repair{
private:
    std::ifstream in_fs; 
    std::ofstream out_fs; 
    uint8_t buf[BUF_SIZE];
    int in_file_len;

public:
    Mp4Repair();
    ~Mp4Repair();

    bool open(char* file);
    FILE_STATUS check(void);
    void close();

private:
    bool boxParse(void);
    bool boxNeedIterate(const char* box_name);
    bool stringCompare(const char* str1, const char* str2, int len);
};