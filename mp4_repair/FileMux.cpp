#ifndef FILEMUX_H
#define FILEMUX_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FileMux.h"

#define STREAM_FRAME_RATE 30
#define DVR_PPS_SPS_LUKE 85
// #define DVR_PPS_SPS_LUKE 80
#define SVC_PPS_SPS_LUKE 80

static  unsigned char Extradata_dvr_luke[DVR_PPS_SPS_LUKE] = {0x0,0x0,0x0,0x1,0x40,0x1,0xc,0x1,0xff,0xff,0x2,0x20,0x0,0x0,0x3,0x0,0xb0,0x0,0x0,0x3,0x0,0x0,0x3,0x0,0x78,0xac,0x9,0x0,0x0,0x0,0x1,0x42,0x1,0x1,0x2,0x20,0x0,0x0,0x3,0x0,0xb0,0x0,0x0,0x3,0x0,0x0,0x3,0x0,0x78,0xa0,0x1,0xe0,0x20,0x6,0xa1,0xf1,0x39,0x6b,0xb9,0x32,0x4b,0xa8,0x2,0xed,0xa,0x12,0x80,0x0,0x0,0x0,0x1,0x44,0x1,0xc0,0xe3,0x2f,0x8,0xb0,0x60,0x30,0x18,0xc,0x73,0x8,0x40,};
// static  unsigned char Extradata_dvr_luke[DVR_PPS_SPS_LUKE] ={0x0,0x0,0x0,0x1,0x40,0x1,0xc,0x1,0xff,0xff,0x2,0x20,0x0,0x0,0x3,0x0,0xb0,0x0,0x0,0x3,0x0,0x0,0x3,0x0,0x5d,0xac,0x9,0x0,0x0,0x0,0x1,0x42 ,0x1,0x1,0x2 ,0x20 ,0x0 ,0x0 ,0x3 ,0x0 ,0xb0 ,0x0 ,0x0 ,0x3 ,0x0 ,0x0 ,0x3 ,0x0 ,0x5d ,0xa0,0x2 ,0x80 ,0x80 ,0x10 ,0x7 ,0xc6 ,0xe5 ,0xae ,0xe4 ,0xc9 ,0x2e ,0xa0 ,0xb ,0xb4 ,0x28 ,0x4a ,0x0 ,0x0 ,0x0 ,0x1 ,0x44 ,0x1 ,0xc0 ,0xe3 ,0x2f ,0x9 ,0x41 ,0x8e ,0x61 ,0x8 };
static  unsigned char Extradata_svc_luke[SVC_PPS_SPS_LUKE] ={0x0,0x0,0x0,0x1,0x40,0x1,0xc,0x1,0xff,0xff,0x2,0x20,0x0,0x0,0x3,0x0,0xb0,0x0,0x0,0x3,0x0,0x0,0x3,0x0,0x5d,0xac,0x9,0x0,0x0,0x0,0x1,0x42 ,0x1,0x1,0x2 ,0x20 ,0x0 ,0x0 ,0x3 ,0x0 ,0xb0 ,0x0 ,0x0 ,0x3 ,0x0 ,0x0 ,0x3 ,0x0 ,0x5d ,0xa0,0x2 ,0x80 ,0x80 ,0x10 ,0x7 ,0xc6 ,0xe5 ,0xae ,0xe4 ,0xc9 ,0x2e ,0xa0 ,0xb ,0xb4 ,0x28 ,0x4a ,0x0 ,0x0 ,0x0 ,0x1 ,0x44 ,0x1 ,0xc0 ,0xe3 ,0x2f ,0x9 ,0x41 ,0x8e ,0x61 ,0x8 };



 FileMux::FileMux(){

 }

FileMux::~FileMux(){

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

    uint8_t* extradata_dvr =(uint8_t*) av_mallocz(DVR_PPS_SPS_LUKE + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(extradata_dvr, Extradata_dvr_luke, DVR_PPS_SPS_LUKE);

    in_videocodecpar->codec_type = AVMEDIA_TYPE_VIDEO; // VIDEO
    in_videocodecpar->codec_id = AV_CODEC_ID_HEVC;
    in_videocodecpar->extradata_size = DVR_PPS_SPS_LUKE;
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

int FileMux::writeFrame(uint8_t* buf, uint32_t len){
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

    if(buf[4] == 0x40 && buf[5] == 0x01){
        // printf("vps\n");
        return 1;
    }else if(buf[4] == 0x42 && buf[5] == 0x01){
        // printf("sps\n");
        return 1;
    }else if(buf[4] == 0x44 && buf[5] == 0x01){
        // printf("pps\n");
        return 1;
    }else if(buf[4] == 0x4E && buf[5] == 0x01){
        // printf("sei\n");
    }else if(buf[4] == 0x26 && buf[5] == 0x01){
        pkt.flags = AV_PKT_FLAG_KEY;
        // printf("idr\n");
    }else if(buf[4] == 0x02 && buf[5] == 0x01){
        // printf("slice\n");
    }else{
        // printf("undefine\n");
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

#endif