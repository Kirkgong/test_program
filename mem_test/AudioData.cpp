#include "AudioData.h"

AudioData::AudioData(){}
AudioData::~AudioData(){}


uint8_t AudioData::buffer[INCIDENT_AUDIO_BUFFER_SIZE];
uint32_t AudioData::head;
uint32_t AudioData::tail;

AudioData* AudioData::malloc(uint32_t length){
    AudioData* data = new AudioData();   
    if(get_capacity() > length){
        data->len = length;
        data->start = data->head;
        data->head = (data->head + length)%INCIDENT_AUDIO_BUFFER_SIZE;
        return data;
    }else{
        return nullptr;
    }
}

uint32_t AudioData::get_capacity(void){
    if(AudioData::head >= AudioData::tail){
        return INCIDENT_AUDIO_BUFFER_SIZE - (head - tail);
    }else{
        return tail - head;
    }
}

bool AudioData::memcpy(AudioData* dst, uint8_t* src, uint32_t len){
    for(uint32_t i=0; i<len; i++){
        buffer[(dst->start + i)%INCIDENT_AUDIO_BUFFER_SIZE] = src[i];
    }
    return true;
}

void AudioData::free(AudioData* data){
    tail = (tail + data->len)%INCIDENT_AUDIO_BUFFER_SIZE;
    delete data;
}