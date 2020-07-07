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
#include "encode_video.h"

CEncodeVideo::CEncodeVideo(){
    memset(&m_encode_video_param, 0x00, sizeof m_encode_video_param);
    on_have_data = nullptr;
    cb_data_ctx = nullptr;
}

int CEncodeVideo::init(cb_data _cb, void* ctx){
    on_have_data = _cb;
    cb_data_ctx = ctx;
    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder_by_name(m_encode_video_param.codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", m_encode_video_param.codec_name);
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    /* put sample parameters */
    c->bit_rate = m_encode_video_param.bit_rate;
    /* resolution must be a multiple of two */
    c->width = m_encode_video_param.width;
    c->height = m_encode_video_param.height;
    /* frames per second */
    c->time_base = (AVRational){1, m_encode_video_param.fps};
    c->framerate = (AVRational){m_encode_video_param.fps, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = m_encode_video_param.gop_size;
    c->max_b_frames = m_encode_video_param.b_frame;
    c->pix_fmt = (enum AVPixelFormat)m_encode_video_param.fmt; 

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

    /* open it */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
        av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
        fprintf(stderr, "Could not open codec: %s\n",errbuf);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    frame->pts = 0;
}
int CEncodeVideo::uninit(){
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

int CEncodeVideo::encode(char* yuv420p_buf, int yuv420p_buf_len){
    char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
    int ret = av_image_fill_linesizes(frame->linesize,(enum AVPixelFormat )frame->format, frame->width);
    if (ret < 0){
        av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
        fprintf(stdout,"#### %s av_image_fill_linesizes failed %s, %d, %d\n", __FUNCTION__, errbuf, frame->format, frame->width);
        exit(0);
    }

    ret = av_image_fill_pointers(frame->data,(enum AVPixelFormat)frame->format, frame->height,
                           (uint8_t*)yuv420p_buf, frame->linesize);

    if (ret < 0){
        av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
        fprintf(stdout,"#### %s av_image_fill_pointers failed %s\n", __FUNCTION__, errbuf);
        exit(0);
    }
    frame->pts++;

    return encode(frame);
}

int CEncodeVideo::encode(AVFrame *_frame){

    int ret;
    
    AVCodecContext *enc_ctx = this->c;

    /* send the frame to the encoder */
    if (frame)
        printf("%s Send frame %3"PRId64"\n",__FUNCTION__, frame->pts);

    ret = avcodec_send_frame(enc_ctx, _frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            return ret;
        }

        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        //fwrite(pkt->data, 1, pkt->size, outfile);
        if(on_have_data && cb_data_ctx){
            on_have_data((char*) pkt->data, pkt->size, cb_data_ctx );
        }
        av_packet_unref(pkt);
    }
    return 0;
}

