#include "AudioData.h"



int main(void){
    AudioData* temp;
    uint8_t test_data[1000];

    for(int i = 0; i<sizeof(test_data); i++){
        test_data[i] = i;
    }

    for(int i=0; i<10000; i++){
        temp = AudioData::malloc(i);
        if(temp != nullptr){
            AudioData::memcpy(temp, test_data, i);
        }else{
            printf("no enough space\n");
        }
        if(temp != nullptr){
            AudioData::free(temp); 
        }
    }
}