#ifndef PRASETSFILE_H
#define PRASETSFILE_H

#include "TSFormat.h"
#include "stdint.h"
#include "stdio.h"
#include "time.h"

#define DUMP_ENABL (1)
#define TIME_STATISTICS (0)
#define FIEL_READ_LEN (TS_STREAM_PACKET_LEN * 1024 * 10)

typedef void (*FRAME_CALLBACK)(uint8_t* buf, uint32_t len, void* args);

class PraseTsFile {
  private:
    FILE* fd;
    char file_name_bak[256];
    uint8_t* buf;
    void* audio_callback_args;
    void* video_callback_args;
    FRAME_CALLBACK audio_callback;
    FRAME_CALLBACK video_callback;

#if DUMP_ENABL
    bool dump_H265_enable;
    bool dump_MP2_enable;
    FILE* dump_H265_fd;
    FILE* dump_MP2_fd;
#endif

#if TIME_STATISTICS
    struct timespec m_time;
    uint32_t read_start_time;
    uint32_t write_start_time;
    uint32_t write_end_time;
#endif

    uint32_t start_time;
    uint32_t video_time;
    uint32_t file_write_offset;
    uint32_t file_read_offset;
    uint32_t file_prase_offset;
    TSFormat* tsformat;

  public:
    PraseTsFile(/* args */);
    ~PraseTsFile();
    void TsFileRead(char* file_name, uint32_t start_time, uint32_t video_time);
    static void frame_ready_callback(void* arg);
    void video_frame_callback_register(FRAME_CALLBACK callback, void* args);
    void audio_frame_callback_register(FRAME_CALLBACK callback, void* args);
};

#endif
