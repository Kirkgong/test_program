#ifndef FILEMUX_H
#define FILEMUX_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FileMux.h"


 FileMux::FileMux(){
    extra_data_buf = (uint8_t*)malloc(EXTRA_DATA_MAX_LEN);
 }

FileMux::~FileMux(){
    if(extra_data_buf){
        free(extra_data_buf);
    }
}

int FileMux::init(void){
    char* filename = (char*)"repair.mp4";

    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
    }
    if (!oc)
        return 1;

    fmt = oc->oformat;

    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        video_st = avformat_new_stream(oc, NULL);
        if (!video_st) {
            fprintf(stderr, "Could not allocate stream\n");
            exit(1);
        }
    }

    /* copy the stream parameters to the muxer */
    AVCodecParameters *in_videocodecpar = avcodec_parameters_alloc();

    uint8_t* extradata_dvr =(uint8_t*) av_mallocz(extra_data_len + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(extradata_dvr, extra_data_buf, extra_data_len);

    in_videocodecpar->codec_type = AVMEDIA_TYPE_VIDEO; // VIDEO
    in_videocodecpar->codec_id = AV_CODEC_ID_HEVC;
    in_videocodecpar->extradata_size = extra_data_len;
    in_videocodecpar->extradata = extradata_dvr;
    in_videocodecpar->format = AV_PIX_FMT_YUV420P;
    in_videocodecpar->profile = 0x2;
    in_videocodecpar->level = 0x800;
    in_videocodecpar->width = 3840;
    in_videocodecpar->height = 1680;
    in_videocodecpar->sample_aspect_ratio = (AVRational){1, 1}; 

    int ret = avcodec_parameters_copy(video_st->codecpar, in_videocodecpar);
    video_st->index = 0;
    video_st->stream_identifier = 1;
    video_st->first_dts =0;
    video_st->r_frame_rate.den = 30;
    video_st->r_frame_rate.num = 1;
    video_st->avg_frame_rate.den = 30;
    video_st->avg_frame_rate.num = 1;
    // video_st->r_frame_rate = (AVRational){1, STREAM_FRAME_RATE};
    // video_st->avg_frame_rate = (AVRational){1, STREAM_FRAME_RATE};
    // video_st->time_base = (AVRational){ 1, 90000};
    video_st->time_base.den = 1000000;
    video_st->time_base.num = 1;
    video_st->id = 0;

    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    // av_dump_format(oc, 0, (const char*)"repair.mp4", 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, (const char*)"repair.mp4", AVIO_FLAG_WRITE);
        if (ret < 0) {
            // fprintf(stderr, "Could not open '%s': %s\n", filename,
                    // av_err2str(ret));
            return 1;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, NULL);
    if (ret < 0) {
        // fprintf(stderr, "Error occurred when opening output file: %s\n",
                // av_err2str(ret));
        return 1;
    }
}

int FileMux::writeFrame(uint8_t* buf, uint32_t len, bool key_frame){
    static int count = 0;
    static int pos = 0;

    AVPacket pkt = {0};
    // av_packet_rescale_ts(&pkt, c->time_base, video_st->time_base);
    pkt.size = len;
    pkt.data = buf;
    pkt.pts = 1000000/30 * count;
    pkt.dts = 1000000/30 * count;
    pkt.duration = 1000000/30;
    pkt.pos = pos;
    pkt.stream_index = AVMEDIA_TYPE_VIDEO;
    pos+=pkt.size;

    
    if(key_frame){
        pkt.flags = AV_PKT_FLAG_KEY;
    }
    count++;

    pkt.stream_index = AVMEDIA_TYPE_VIDEO;
    // log_packet(oc, &pkt);
    int ret = av_interleaved_write_frame(oc, &pkt);
    av_packet_unref(&pkt);
    if (ret < 0) {
        // fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
        // exit(1);
    }
}


void FileMux::close(void){
    av_write_trailer(oc);
    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&oc->pb);

    /* free the stream */
    avformat_free_context(oc);
}

void FileMux::writeExtraData(uint8_t* data, uint32_t len){
    if(len + extra_data_len < EXTRA_DATA_MAX_LEN){
        memcpy(&extra_data_buf[extra_data_len], data, len);
        extra_data_len+=len;
    }else{
        printf("Extra data len bigger than max extra data len\n");
    }
}

#endif