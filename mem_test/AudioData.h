#include "definition.h"

#define INCIDENT_AUDIO_BUFFER_SIZE      500*1024

class AudioData{
    private:
        static uint8_t buffer[INCIDENT_AUDIO_BUFFER_SIZE];
        static uint32_t head;
        static uint32_t tail;

    public:
        uint32_t len;
        uint32_t start;

        AudioData();
        ~AudioData();
        static AudioData* malloc(uint32_t length);
        static bool memcpy(AudioData* dst, uint8_t* src, uint32_t len);
        static uint32_t get_capacity(void);
        static void free(AudioData* data);
};