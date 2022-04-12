#include "Mp4Repair.h"
#include <iostream>

using namespace std;

#define ITERRATE_BOX_NUM       7
#define BOX_HEAD_LEN           8
#define BOX_NAME_LEN           4
#define SAMPLE_HEAD_LEN        4
#define NALU_HEAD_LEN          4
#define NALU_TYPE_LEN          3
#define AUDIO_SAMPLE_LEN       576


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
    if(stringCompare((char*)&buf[BOX_NAME_LEN], "mdat", BOX_NAME_LEN)){
        *status = FILE_STATUR_ABNORMAL;
        mdat_index = in_fs.tellg();
        if(box_len == 0 || box_len == 1){
            mdat_len = in_file_len - mdat_index;
        }else{
            mdat_len = box_len - BOX_HEAD_LEN;
        }
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
    static NAUL_TYPE naul_type = NAUL_TYPE_NONE; 

    if(mdat_len == 0){
        printf("Please check file first.");
        return FILE_STATUR_NORMAL;
    }
    open(file);
    in_fs.seekg(0, ios::end);
    in_file_len = in_fs.tellg();
    in_fs.seekg(0, ios::beg);
    in_fs.seekg(mdat_index, ios::beg);

    while(in_fs.tellg() != mdat_index + mdat_len){
        in_fs.read((char*)&buf[buf_index], SAMPLE_HEAD_LEN + NALU_TYPE_LEN);
        // for(int i=0; i<SAMPLE_HEAD_LEN + NALU_TYPE_LEN; i++){
            // printf("0x%02x ", buf[buf_index + i]);
        // }
        // printf("\n");

        if(buf[buf_index] == 0xFF){
            // audio sample
            if(buf_index != 0){
                write(buf, buf_index, naul_type);
                naul_type = NAUL_TYPE_NONE; 
            }
            in_fs.seekg(AUDIO_SAMPLE_LEN - (SAMPLE_HEAD_LEN + NALU_TYPE_LEN), ios::cur);
        }else{
            uint32_t smaple_len = (buf[buf_index +  0]<<24) + 
                                  (buf[buf_index +  1] << 16) + 
                                  (buf[buf_index +  2] << 8) + 
                                  (buf[buf_index +  3] << 0);
            // video sample
            if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x26 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_index + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    if(buf_index != 0){
                        write(buf, buf_index, naul_type);
                    }
                }
                naul_type = NAUL_TYPE_IDR;
            }else if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x02 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_index + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    if(buf_index != 0){
                        write(buf, buf_index, naul_type);
                    }
                } 
                naul_type = NAUL_TYPE_SLICE;
            }else if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x44 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                naul_type = NAUL_TYPE_PPS;
            }

            in_fs.seekg(-NALU_TYPE_LEN, ios::cur);

            buf[buf_index + 0] = 0x00;
            buf[buf_index + 1] = 0x00;
            buf[buf_index + 2] = 0x00;
            buf[buf_index + 3] = 0x01;
            buf_index += SAMPLE_HEAD_LEN;

            in_fs.read((char*)&buf[buf_index], smaple_len);
            buf_index += smaple_len;
        }
    }
    file_mux.close();
}


void Mp4Repair::write( uint8_t* data, uint32_t len, NAUL_TYPE type){
    if(type == NAUL_TYPE_PPS){
        file_mux.writeExtraData(buf, buf_index);
        file_mux.init();
        file_mux.writeFrame(buf, buf_index, true);
        buf_index = 0;
    }else if(type == NAUL_TYPE_SLICE){
        file_mux.writeFrame(buf, buf_index, false);
        buf_index = 0;
    }else if(type == NAUL_TYPE_IDR){
        file_mux.writeFrame(buf, buf_index, true);
        buf_index = 0;
    }
}
