/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * video encoding with libavcodec API example
 *
 * @example encode_video.c
 */

extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

struct EncodeVideoParam{
    int         bit_rate;
    short       width;
    short       height;
    unsigned char fps;
    unsigned char gop_size;
    unsigned char b_frame;
    unsigned char fmt;
    char        codec_name[32];
};

class CEncodeVideo{
public:
    typedef void (*cb_data)(char* buf, int buf_len, void* ctx);

    CEncodeVideo();
    int  init(cb_data _cb, void* ctx);
    int  uninit();
    int  encode(char* yuv420p_buf, int yuv420p_buf_len);
    int  encode(AVFrame* frame);

    EncodeVideoParam m_encode_video_param;
    // FILE *f;
private:
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y;
    AVFrame *frame;
    AVPacket *pkt;
    //uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    
    cb_data on_have_data;
    void*   cb_data_ctx;
};

