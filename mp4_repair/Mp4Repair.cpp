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
    mdat_box_index = 0;
    mdat_box_len = 0;
    buf = (uint8_t*)malloc(BUF_SIZE);
}

Mp4Repair::~Mp4Repair(){
    if(buf){
        free(buf); 
    }
}

ifstream* Mp4Repair::openFile(char* file){
    ifstream* fs = new(std::ifstream);    
    fs->open(file, ios::in | ios::binary);
    return fs;
}

void Mp4Repair::closeFile(ifstream* fs){
    fs->close();
    delete fs;
    fs = nullptr;
}

FILE_STATUS Mp4Repair::process(char* file){
    FILE_STATUS status = checkFile(file);

    if(status == FILE_STATUR_NORMAL){
        cout << "File status is normal.Don't need repair." << endl;
    }else if(status == FILE_STATUR_DAMAGE){
        cout << "File status is undefine.Can't repair." << endl;
    }else if(status == FILE_STATUR_ABNORMAL){
        status  = repair(file);
        if(status == FILE_STATUR_REPAIR_FAIL){
            cout << "Repair fail." << endl;
        }else if(status == FILE_STATUR_REPAIR_SUCCESS){
            cout << "Repair success." << endl;
        }
    }

    return status;
}

FILE_STATUS Mp4Repair::checkFile(char* file){
    std::ifstream* fs; 
    fs = openFile(file);
    FILE_STATUS status = FILE_STATUR_DAMAGE; 
    boxParse(fs, &status, getFileLen(fs, 0));
    closeFile(fs);
    return status;
}

bool Mp4Repair::boxParse(std::ifstream* fs, FILE_STATUS* status, uint32_t file_len){
    if(file_len - fs->tellg() < BOX_HEAD_LEN){
        return false;
    }
    fs->read((char*)buf, BOX_HEAD_LEN);

    uint32_t box_len = (buf[0]<<24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
    if(stringCompare((char*)&buf[BOX_NAME_LEN], "mdat", BOX_NAME_LEN)){
        *status = FILE_STATUR_ABNORMAL;
        setMdatBoxIndex(fs->tellg());
        if(box_len == 0 || box_len == 1){
            setMdatBoxLen(file_len - getMdatBoxIndex());
        }else{
            setMdatBoxLen(box_len - BOX_HEAD_LEN);
        }
    }else if(stringCompare((char*)&buf[BOX_NAME_LEN], "moov", BOX_NAME_LEN)){
        *status = FILE_STATUR_NORMAL;
    }

    if(file_len - fs->tellg() < box_len - BOX_HEAD_LEN){
        return false;
    }

    if(!boxNeedIterate((char*)&buf[BOX_NAME_LEN])){
        fs->seekg(box_len - BOX_HEAD_LEN, ios::cur);
    }

    if(fs->tellg() != file_len){
        return boxParse(fs, status, file_len);
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
    FileMux* file_mux = new(FileMux);
    uint32_t buf_len = 0;
    NAUL_TYPE naul_type = NAUL_TYPE_NONE; 

    std::ifstream*fs = openFile(file);
    fs->seekg(getMdatBoxIndex(), ios::beg);

    while(fs->tellg() != getMdatBoxIndex() + getMdatBoxLen()){
        fs->read((char*)&buf[buf_len], SAMPLE_HEAD_LEN + NALU_TYPE_LEN);

        if(buf[buf_len] == 0xFF){
            // audio sample
            write(file_mux, buf, buf_len, naul_type);
            buf_len = 0;

            naul_type = NAUL_TYPE_NONE; 
            fs->seekg(AUDIO_SAMPLE_LEN - (SAMPLE_HEAD_LEN + NALU_TYPE_LEN), ios::cur);
        }else{
            uint32_t smaple_len = (buf[buf_len +  0]<<24) + 
                                  (buf[buf_len +  1] << 16) + 
                                  (buf[buf_len +  2] << 8) + 
                                  (buf[buf_len +  3] << 0);
            // video sample
            if(buf[buf_len + SAMPLE_HEAD_LEN] == 0x26 && buf[buf_len + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_len + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    write(file_mux, buf, buf_len, naul_type);
                    buf_len = 0;
                }
                naul_type = NAUL_TYPE_IDR;
            }else if(buf[buf_len + SAMPLE_HEAD_LEN] == 0x02 && buf[buf_len + SAMPLE_HEAD_LEN + 1] == 0x01){
                if(buf[buf_len + SAMPLE_HEAD_LEN + 2]  & 0x80){
                    write(file_mux, buf, buf_len, naul_type);
                    buf_len = 0;
                } 
                naul_type = NAUL_TYPE_SLICE;
            }else if(buf[buf_len + SAMPLE_HEAD_LEN] == 0x44 && buf[buf_len + SAMPLE_HEAD_LEN + 1] == 0x01){
                naul_type = NAUL_TYPE_PPS;
            }

            fs->seekg(-NALU_TYPE_LEN, ios::cur);

            buf[buf_len + 0] = 0x00;
            buf[buf_len + 1] = 0x00;
            buf[buf_len + 2] = 0x00;
            buf[buf_len + 3] = 0x01;
            buf_len += SAMPLE_HEAD_LEN;

            fs->read((char*)&buf[buf_len], smaple_len);
            buf_len += smaple_len;
        }
    }
    closeFile(fs);
    file_mux->close();
    delete file_mux;
    return FILE_STATUR_REPAIR_SUCCESS;
}

void Mp4Repair::write(FileMux* mux, uint8_t* data, uint32_t len, NAUL_TYPE type){
    if(type == NAUL_TYPE_PPS){
        mux->writeExtraData(data, len);
        mux->init();
        mux->writeFrame(data, len, true);
    }else if(type == NAUL_TYPE_SLICE){
        mux->writeFrame(data, len, false);
    }else if(type == NAUL_TYPE_IDR){
        mux->writeFrame(data, len, true);
    }
}

void  Mp4Repair::setMdatBoxIndex(uint32_t index){
    mdat_box_index = index;
}

void  Mp4Repair::setMdatBoxLen(uint32_t len){
    mdat_box_len = len;
}

uint32_t  Mp4Repair::getMdatBoxIndex(void){
    return mdat_box_index;
}

uint32_t  Mp4Repair::getMdatBoxLen(void){
    return mdat_box_len;
}
