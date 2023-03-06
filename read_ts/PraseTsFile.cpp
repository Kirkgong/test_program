#include "PraseTsFile.h"
#include "stdlib.h"

PraseTsFile::PraseTsFile()
{
    tsformat = new TSFormat();
    tsformat->set_frame_ready_callback(frame_ready_callback);
    buf = (uint8_t*)malloc(FIEL_READ_LEN);

    audio_callback = nullptr;
    video_callback = nullptr;

#if DUMP_ENABL
    dump_H265_enable = false;
    dump_MP2_enable = false;
#endif

#if TIME_STATISTICS
    read_start_time = 0;
    write_start_time = 0;
    write_end_time = 0;
#endif
    file_write_offset = 0;
    file_read_offset = 0;
    file_prase_offset = 0;

    memset(file_name_bak, 0, sizeof(file_name_bak));
}

PraseTsFile::~PraseTsFile()
{
    free(buf);
    delete tsformat;
}

void PraseTsFile::frame_ready_callback(void* arg)
{
    static uint32_t count = 0;

    Frame_Struct* frame = (Frame_Struct*)arg;
    PraseTsFile* prase_ts_file = (PraseTsFile*)frame->Private;

    if (frame->pts >= prase_ts_file->start_time && frame->pts <= prase_ts_file->start_time + prase_ts_file->video_time) {
        if (frame->stream_type == 0xE0) {
#if TIME_STATISTICS
            if (prase_ts_file->write_start_time == 0) {
                clock_gettime(CLOCK_MONOTONIC, &prase_ts_file->m_time);
                prase_ts_file->write_start_time = prase_ts_file->m_time.tv_nsec / (1000 * 1000) + prase_ts_file->m_time.tv_sec;
                printf("write_start_time = %d\n", prase_ts_file->write_start_time);
            }
#endif

#if DUMP_ENABL
            if (prase_ts_file->dump_H265_enable) {
                fwrite(frame->buf, 1, frame->frame_len, prase_ts_file->dump_H265_fd);
            }
#endif
            if (prase_ts_file->video_callback != nullptr) {
                prase_ts_file->video_callback(frame->buf, frame->frame_len, prase_ts_file->video_callback_args);
            }
        }
        else {
#if DUMP_ENABL
            if (prase_ts_file->dump_MP2_enable) {
                fwrite(frame->buf, 1, frame->frame_len, prase_ts_file->dump_MP2_fd);
            }
#endif
            if (prase_ts_file->audio_callback != nullptr) {
                prase_ts_file->audio_callback(frame->buf, frame->frame_len, prase_ts_file->audio_callback_args);
            }
        }
    }

    prase_ts_file->file_write_offset = prase_ts_file->file_prase_offset;

    if (prase_ts_file->start_time > 2 * 30 * 3000 + frame->pts) {
        uint32_t delta_sec = (prase_ts_file->start_time - frame->pts) / 30 / 3000 - 2;
        prase_ts_file->file_write_offset += (2 * 1024 * 1024 / 8 * delta_sec);
        prase_ts_file->file_write_offset = prase_ts_file->file_write_offset / TS_STREAM_PACKET_LEN * TS_STREAM_PACKET_LEN;
    }
}

void PraseTsFile::video_frame_callback_register(FRAME_CALLBACK callback, void* args)
{
    video_callback_args = args;
    video_callback = callback;
}

void PraseTsFile::audio_frame_callback_register(FRAME_CALLBACK callback, void* args)
{
    audio_callback_args = args;
    audio_callback = callback;
}

void PraseTsFile::TsFileRead(char* file_name, uint32_t start_time, uint32_t video_time)
{
    size_t num;

    fd = fopen(file_name, "rb");
    if (fd != nullptr) {
        if (strcmp(file_name, this->file_name_bak) != 0) {
            printf("read %d s from %s at %d\n", video_time/90000, file_name, start_time/90000);
            memset(this->file_name_bak, 0, sizeof(this->file_name_bak));
            memcpy(this->file_name_bak, file_name, strlen(file_name));
            file_write_offset = 0;
            tsformat->clear_pmt();
            tsformat->clear_len();
        }

        file_read_offset = 0;
        file_prase_offset = 0;

        this->start_time = start_time;
        this->video_time = video_time;

#if DUMP_ENABL
        dump_H265_fd = fopen("dump.h265", "ab");
        dump_MP2_fd = fopen("dump.mp2", "ab");
        dump_H265_enable = true;
        dump_MP2_enable = true;
#endif

#if TIME_STATISTICS
        clock_gettime(CLOCK_MONOTONIC, &m_time);
        read_start_time = m_time.tv_nsec / (1000 * 1000) + m_time.tv_sec;
        printf("read_start_time = %d\n", read_start_time);
#endif

        while (1) {
            if (file_write_offset > file_prase_offset) {
                fseek(fd, file_write_offset, SEEK_SET);
                file_read_offset = file_write_offset;
                file_prase_offset = file_write_offset;
                tsformat->clear_len();
            }

            if (file_prase_offset == file_read_offset) {
                num = fread(buf, 1, FIEL_READ_LEN, fd);
                file_read_offset += num;
            }
            else if (file_prase_offset > file_read_offset) {
                printf("file_prase_offset error\n");
            }

            if (file_read_offset < TS_STREAM_PACKET_LEN + file_prase_offset) {
                break;
            }
            tsformat->Parse_Packet(&buf[file_prase_offset + num - file_read_offset], this);
            file_prase_offset += TS_STREAM_PACKET_LEN;
        }

#if DUMP_ENABL
        fclose(dump_H265_fd);
        fclose(dump_MP2_fd);
#endif

        fclose(fd);

#if TIME_STATISTICS
        clock_gettime(CLOCK_MONOTONIC, &m_time);
        write_end_time = m_time.tv_nsec / (1000 * 1000) + m_time.tv_sec;
        printf("write_end_time = %d\n", write_end_time);
        printf("skip time = %d\n", write_start_time - read_start_time);
        printf("write time = %d\n", write_end_time - write_start_time);
#endif
        // printf("file_prase_offset = %d\n", file_prase_offset);
    }
}
