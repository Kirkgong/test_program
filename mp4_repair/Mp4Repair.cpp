#include "Mp4Repair.h"
#include <iostream>
#include "FileMux.h"

using namespace std;

#define ITERRATE_BOX_NUM       7
#define BOX_HEAD_LEN           8
#define BOX_NAME_LEN           4
#define SAMPLE_HEAD_LEN        4
#define NALU_HEAD_LEN          4
#define NALU_TYPE_LEN          2


const char* iterate_boxs_name[ITERRATE_BOX_NUM] = {
    "moov",
    "trak",
    "meta",
    "mdia",
    "minf",
    "dinf",
    "stbl",
};

Mp4Repair::Mp4Repair(){
}

Mp4Repair::~Mp4Repair(){}


bool Mp4Repair::open(char* file){
    in_fs.open(file, ios::in | ios::binary);
}

void Mp4Repair::close(void){
    in_fs.close();
}

FILE_STATUS Mp4Repair::check(char* file){
    open(file);
    in_fs.seekg(0, ios::end);
    in_file_len = in_fs.tellg();
    in_fs.seekg(0, ios::beg);
    FILE_STATUS status = FILE_STATUR_DAMAGE; 
    boxParse(&status);
    close();
    return status;
}

bool Mp4Repair::boxParse(FILE_STATUS* status){
    if(in_file_len - in_fs.tellg() < BOX_HEAD_LEN){
        return false;
    }
    in_fs.read((char*)buf, BOX_HEAD_LEN);

    uint32_t box_len = (buf[0]<<24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
    // cout << "box_type " << buf[4] << buf[5] << buf[6] << buf[7] << endl;
    // cout << "box_len = " << box_len << endl;
    if(stringCompare((char*)&buf[BOX_NAME_LEN], "mdat", BOX_NAME_LEN)){
        *status = FILE_STATUR_ABNORMAL;
        mdat_index = in_fs.tellg();
        mdat_len = box_len - BOX_HEAD_LEN;
    }else if(stringCompare((char*)&buf[BOX_NAME_LEN], "moov", BOX_NAME_LEN)){
        *status = FILE_STATUR_NORMAL;
    }

    if(in_file_len - in_fs.tellg() < box_len - BOX_HEAD_LEN){
        return false;
    }

    if(!boxNeedIterate((char*)&buf[BOX_NAME_LEN])){
        in_fs.seekg(box_len - BOX_HEAD_LEN, ios::cur);
    }

    if(in_fs.tellg() != in_file_len){
        return boxParse(status);
    }else{
        return true;
    }
}

bool Mp4Repair::boxNeedIterate(const char* box_name){
    for(int i=0; i<ITERRATE_BOX_NUM; i++){
        if(stringCompare(box_name, iterate_boxs_name[i], 4)){
            return true;
        }
    }
    return false;
}

bool Mp4Repair::stringCompare(const char* str1, const char* str2, int len){
    for(int i=0; i<len; i++){
        if(str1[i] != str2[i]){
            return false;
        }
    }
    return true;
}

FILE_STATUS Mp4Repair::repair(char* file){
    open(file);
    in_fs.seekg(0, ios::end);
    in_file_len = in_fs.tellg();
    in_fs.seekg(0, ios::beg);
    in_fs.seekg(mdat_index, ios::beg);

    FileMux file_mux;
    file_mux.init();

    while(in_fs.tellg() != mdat_index + mdat_len){
            in_fs.read((char*)buf, SAMPLE_HEAD_LEN);
            for(int i=0; i<SAMPLE_HEAD_LEN; i++){
                printf("0x%02x ", buf[i]);
            }
            printf("\n");

            if(buf[0] == 0xFF){
                // audio sample
                in_fs.seekg(576 - SAMPLE_HEAD_LEN, ios::cur);
            }else{
                // video sample
                uint32_t smaple_len = (buf[0]<<24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
                in_fs.read((char*)&buf[NALU_HEAD_LEN], smaple_len);

                if(buf[4] == 0x40 && buf[5] == 0x01){
                    printf("vps\n");
                    continue;
                }else if(buf[4] == 0x42 && buf[5] == 0x01){
                    printf("sps\n");
                    continue;
                }else if(buf[4] == 0x44 && buf[5] == 0x01){
                    printf("pps\n");
                    continue;
                }else if(buf[4] == 0x4E && buf[5] == 0x01){
                    printf("sei\n");
                    continue;
                }else if(buf[4] == 0x26 && buf[5] == 0x01){
                    printf("idr\n");
                }else if(buf[4] == 0x02 && buf[5] == 0x01){
                    printf("slice\n");
                }else{
                    printf("undefine\n");
                    continue;
                }
            }
    }
    file_mux.close();
}
