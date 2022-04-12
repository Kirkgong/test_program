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
    buf = (uint8_t*)malloc(BUF_SIZE);
    file_mux = new(FileMux);
    buf_index = 0;
}

Mp4Repair::~Mp4Repair(){
    if(buf){
        free(buf); 
    }
    if(file_mux){
        delete file_mux;
        file_mux = nullptr;
    }
}

ifstream* Mp4Repair::open(char* file){
    ifstream* fs = new(std::ifstream);    
    fs->open(file, ios::in | ios::binary);
    return fs;
}

void Mp4Repair::close(ifstream* fs){
    fs->close();
    delete fs;
    fs = nullptr;
}

FILE_STATUS Mp4Repair::check(char* file){
    std::ifstream* fs; 
    fs = open(file);
    in_file_len = getFileLen(fs, 0);
    // fs->seekg(0, ios::end);
    // in_file_len = fs->tellg();
    // fs->seekg(0, ios::beg);
    FILE_STATUS status = FILE_STATUR_DAMAGE; 
    boxParse(fs, &status);
    close(fs);
    return status;
}

bool Mp4Repair::boxParse(std::ifstream* fs, FILE_STATUS* status){
    if(in_file_len - fs->tellg() < BOX_HEAD_LEN){
        return false;
    }
    fs->read((char*)buf, BOX_HEAD_LEN);

    uint32_t box_len = (buf[0]<<24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
    if(stringCompare((char*)&buf[BOX_NAME_LEN], "mdat", BOX_NAME_LEN)){
        *status = FILE_STATUR_ABNORMAL;
        mdat_index = fs->tellg();
        if(box_len == 0 || box_len == 1){
            mdat_len = in_file_len - mdat_index;
        }else{
            mdat_len = box_len - BOX_HEAD_LEN;
        }
    }else if(stringCompare((char*)&buf[BOX_NAME_LEN], "moov", BOX_NAME_LEN)){
        *status = FILE_STATUR_NORMAL;
    }

    if(in_file_len - fs->tellg() < box_len - BOX_HEAD_LEN){
        return false;
    }

    if(!boxNeedIterate((char*)&buf[BOX_NAME_LEN])){
        fs->seekg(box_len - BOX_HEAD_LEN, ios::cur);
    }

    if(fs->tellg() != in_file_len){
        return boxParse(fs, status);
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

uint32_t Mp4Repair::getFileLen(std::ifstream* fs, uint32_t seek){
    uint32_t ret;
    fs->seekg(0, ios::end);
    ret = fs->tellg();
    fs->seekg(seek, ios::beg);
    return ret;
}

FILE_STATUS Mp4Repair::repair(char* file){
    std::ifstream* fs; 
    NAUL_TYPE naul_type = NAUL_TYPE_NONE; 

    if(mdat_len == 0){
        printf("Please check file first.");
        return FILE_STATUR_NORMAL;
    }

    fs = open(file);
    in_file_len = getFileLen(fs, mdat_index);

    while(fs->tellg() != mdat_index + mdat_len){
        fs->read((char*)&buf[buf_index], SAMPLE_HEAD_LEN + NALU_TYPE_LEN);

        if(buf[buf_index] == 0xFF){
            // audio sample
            write(buf, buf_index, naul_type);
            naul_type = NAUL_TYPE_NONE; 
            fs->seekg(AUDIO_SAMPLE_LEN - (SAMPLE_HEAD_LEN + NALU_TYPE_LEN), ios::cur);
        }else{
            uint32_t smaple_len = (buf[buf_index +  0]<<24) + 
                                  (buf[buf_index +  1] << 16) + 
                                  (buf[buf_index +  2] << 8) + 
                                  (buf[buf_index +  3] << 0);
            // video sample
            if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x26 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_index + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    write(buf, buf_index, naul_type);
                }
                naul_type = NAUL_TYPE_IDR;
            }else if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x02 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_index + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    write(buf, buf_index, naul_type);
                } 
                naul_type = NAUL_TYPE_SLICE;
            }else if(buf[buf_index + SAMPLE_HEAD_LEN] == 0x44 && buf[buf_index + SAMPLE_HEAD_LEN + 1] == 0x01){
                naul_type = NAUL_TYPE_PPS;
            }

            fs->seekg(-NALU_TYPE_LEN, ios::cur);

            buf[buf_index + 0] = 0x00;
            buf[buf_index + 1] = 0x00;
            buf[buf_index + 2] = 0x00;
            buf[buf_index + 3] = 0x01;
            buf_index += SAMPLE_HEAD_LEN;

            fs->read((char*)&buf[buf_index], smaple_len);
            buf_index += smaple_len;
        }
    }
    close(fs);
    file_mux->close();
}

void Mp4Repair::write( uint8_t* data, uint32_t len, NAUL_TYPE type){
    if(type == NAUL_TYPE_PPS){
        file_mux->writeExtraData(buf, buf_index);
        file_mux->init();
        file_mux->writeFrame(buf, buf_index, true);
        buf_index = 0;
    }else if(type == NAUL_TYPE_SLICE){
        file_mux->writeFrame(buf, buf_index, false);
        buf_index = 0;
    }else if(type == NAUL_TYPE_IDR){
        file_mux->writeFrame(buf, buf_index, true);
        buf_index = 0;
    }
}
