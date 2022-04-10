#ifndef FILEMUX_H
#define FILEMUX_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FileMux.h"

#define STREAM_FRAME_RATE 30

extern "C"{
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}


 FileMux::FileMux(){

 }

FileMux::~FileMux(){

}


int FileMux::init(void){
    char* filename = (char*)"repair.mp4";
    AVFormatContext *oc;
    AVOutputFormat *fmt;
    AVStream* video_st = { 0 }, audio_st = { 0 };
    AVDictionary *opt = NULL;

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
        video_st->id = oc->nb_streams-1;
        video_st->time_base = (AVRational){ 1, STREAM_FRAME_RATE};
    }

    /* copy the stream parameters to the muxer */
    AVCodecContext *c;
    int ret = avcodec_parameters_from_context(video_st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    av_dump_format(oc, 0, (const char*)"repair.mp4", 1);

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
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        // fprintf(stderr, "Error occurred when opening output file: %s\n",
                // av_err2str(ret));
        return 1;
    }

    AVPacket pkt = { 0 };
    av_packet_rescale_ts(&pkt, c->time_base, video_st->time_base);

    pkt.stream_index = video_st->index;
    // log_packet(oc, &pkt);
    ret = av_interleaved_write_frame(oc, &pkt);
    av_packet_unref(&pkt);
    if (ret < 0) {
        // fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
        // exit(1);
    }
    
    av_write_trailer(oc);
    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&oc->pb);

    /* free the stream */
    avformat_free_context(oc);
}













#endif