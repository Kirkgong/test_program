#include "Mp4Repair.h"
#include <iostream>

using namespace std;

#define ITERRATE_BOX_NUM       7
#define BOX_HEAD_LEN           8
#define BOX_NAME_LEN           4


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

FILE_STATUS Mp4Repair::check(void){
    in_fs.seekg(0, ios::end);
    in_file_len = in_fs.tellg();
    cout << "filelen = " << in_file_len<<endl;
    in_fs.seekg(0, ios::beg);
    boxParse();
}


bool Mp4Repair::boxParse(void){
    if(in_file_len - in_fs.tellg() < BOX_HEAD_LEN){
        return false;
    }
    in_fs.read((char*)buf, BOX_HEAD_LEN);
    // for(int i=0; i<BOX_HEAD_LEN; i++){
    //     printf("0x%02x ", buf[i]);
    // }
    // printf("\n");

    uint32_t box_len = (buf[0]<<24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
    cout << "box_type " << buf[4] << buf[5] << buf[6] << buf[7] << endl;
    cout << "box_len = " << box_len << endl;

    if(in_file_len - in_fs.tellg() < box_len - BOX_HEAD_LEN){
        return false;
    }

    if(boxNeedIterate((char*)&buf[BOX_NAME_LEN])){
        if(!boxParse()){
            return false;
        }
    }else{
        if(in_fs.tellg() != in_file_len){
            in_fs.seekg(box_len - BOX_HEAD_LEN, ios::cur);
            boxParse();
        }
    }
    return true;
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
