#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stdint.h"

#define __STDC_CONSTANT_MACROS

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

public:
    FileMux();
    ~FileMux();
    int init(void);
    int writeFrame(uint8_t* buf, uint32_t len, bool key_frame);
    void close(void);
};

