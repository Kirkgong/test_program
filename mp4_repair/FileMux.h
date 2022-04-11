#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stdint.h"

#define __STDC_CONSTANT_MACROS
#define EXTRA_DATA_MAX_LEN  128

extern "C"{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

class FileMux{
private:
    AVFormatContext *oc;
    AVStream* video_st;
    AVStream* audio_st;
    AVOutputFormat *fmt;
    uint8_t* extra_data_buf;
    uint32_t extra_data_len;

public:
    FileMux();
    ~FileMux();
    int init(void);
    int writeFrame(uint8_t* buf, uint32_t len, bool key_frame);
    void writeExtraData(uint8_t* data, uint32_t len);
    void close(void);
};

