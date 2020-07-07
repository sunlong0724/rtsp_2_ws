#pragma once
#include "ithread.h"

extern "C"{
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
}


class CDemuxAndDec : public IThread{

public:

    typedef void (*cb_info)(int w, int h, void* ctx);
    typedef void (*cb_data)(char* buf, int buf_len, void* ctx);

    void setup_cb_info(cb_info _cb, void* ctx);
    void setup_cb_data(cb_data _cb, void* ctx);

    void start(const char* _src_filename);
    virtual void run();

    int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    int output_video_frame(AVFrame *frame);
private:

    cb_info             dump_info_cb;
    void                *cb_info_ctx = NULL;
    cb_data             on_have_data;
    void                *cb_data_ctx = NULL;

    AVFormatContext     *fmt_ctx = NULL;
    AVCodecContext      *video_dec_ctx = NULL;
    int                 width, height;
    enum AVPixelFormat  pix_fmt;
    AVStream            *video_stream = NULL;
    const char          *src_filename = NULL;
    
    uint8_t             *video_dst_data[4] = {NULL};
    int                 video_dst_linesize[4];
    int                 video_dst_bufsize;
    
    int                 video_stream_idx = -1;
    AVFrame             *frame = NULL;
    AVPacket            pkt;
    int                 video_frame_count = 0;

};


