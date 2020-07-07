#include <stdio.h>
#include <stdlib.h>

#include "app.h"

typedef struct {
	unsigned char magic[4];
	unsigned short width;
	unsigned short height;
} jsmpeg_header_t;

int swap_int32(int in) {
	return ((in>>24)&0xff) |
		((in<<8)&0xff0000) |
		((in>>8)&0xff00) |
		((in<<24)&0xff000000);
}

int swap_int16(short in) {
	return ((in>>8)&0xff) | ((in<<8)&0xff00);
}

// Proxies for app_on_* callbacks
void on_connect(server_t *server, libwebsocket *socket) { fprintf(stdout, "%s \n",__FUNCTION__); app_on_connect((app_t *)server->user, socket); }
void on_message(server_t *server, libwebsocket *socket, void *data, size_t len) { app_on_message((app_t *)server->user, socket, data, len); }
void on_close(server_t *server, libwebsocket *socket) { app_on_close((app_t *)server->user, socket); }



static void encode_video_cb_data( char* buf, int buf_len, void* ctx ){
    fprintf(stdout, "%s %d\n", __FUNCTION__, buf_len);
    app_t *self = (app_t*) ctx;
    self->frame->size = swap_int32(sizeof(jsmpeg_frame_t) + buf_len);
    //memcpy( self->frame->data, buf, buf_len );
    self->frame->data = buf;
    //fprintf(stdout, "1%s %d\n", __FUNCTION__, buf_len);
    server_broadcast(self->server, self->frame, sizeof(jsmpeg_frame_t) +buf_len, server_type_binary);

   // fprintf(stdout, "2%s %d\n", __FUNCTION__, buf_len);
	server_update(self->server);
}

static void demux_dec_cb_info(int w, int h, void* ctx){
    fprintf(stdout,"w:%d,h:%d\n", w, h);
    app_t *self = (app_t*) ctx;

    self->encoder = new CEncodeVideo;
    CEncodeVideo *enc = self->encoder;
    enc->m_encode_video_param.width = w;
    enc->m_encode_video_param.height = h;
    enc->m_encode_video_param.bit_rate = 400000;
    enc->m_encode_video_param.b_frame = 0;
    strcpy(enc->m_encode_video_param.codec_name, "mpeg1video");
    enc->m_encode_video_param.fmt = (enum AVPixelFormat) AV_PIX_FMT_YUV420P;
    enc->m_encode_video_param.fps = 25;
    enc->m_encode_video_param.gop_size = 25;

    //enc->f = fopen("1.mp4", "wb");
    enc->init( encode_video_cb_data, self  );

    self->frame = new jsmpeg_frame_t; //(jsmpeg_frame_t *)malloc(APP_FRAME_BUFFER_SIZE);
	self->frame->type = jsmpeg_frame_type_video;
	self->frame->size = 0;

    self->server = server_create(self->lws_port, APP_FRAME_BUFFER_SIZE, on_connect, on_message, on_close);
	if( !self->server ) {
		printf("Error: could not create Server; try using another port\n");
        exit(0);
	}


	self->server->user = self; // Set the app as user data, so we can access it in callbacks


    fprintf(stdout,"%s %x %x\n", __FUNCTION__, self->server, self->server->on_connect);
}

static void demux_dec_cb_data(char* buf, int buf_len, void* ctx){
    app_t *self = (app_t*) ctx;
    self->encoder->encode( buf, buf_len );
}


app_t *app_create_and_start(const char* rtsp_url, int lws_port) {
	app_t *self = (app_t *)malloc(sizeof(app_t));
	memset(self, 0, sizeof(app_t));

    self->lws_port = lws_port;
	
    self->demux_and_dec = new CDemuxAndDec;
    self->demux_and_dec->setup_cb_info(demux_dec_cb_info, self);
    self->demux_and_dec->setup_cb_data(demux_dec_cb_data, self );
    self->demux_and_dec->start( rtsp_url );

	return self;
}

void app_destroy(app_t *self) { 
    if (self){
        self->demux_and_dec->stop();
        delete self->demux_and_dec;
        delete self->encoder;
	    server_destroy(self->server);
	    free(self);
    }
}

void app_on_connect(app_t *self, libwebsocket *socket) {
    fprintf(stdout,"%s \n ", __FUNCTION__);
	printf("\n###### client connected: %s\n", server_get_client_address(self->server, socket));

	jsmpeg_header_t header = {		
		{'j','s','m','p'}, 
		(unsigned short)swap_int16(self->encoder->m_encode_video_param.width), (unsigned short)swap_int16(self->encoder->m_encode_video_param.height)
	};
	server_send(self->server, socket, &header, sizeof(header), server_type_binary);
}

void app_on_close(app_t *self, libwebsocket *socket) {
	printf("\nclient disconnected: %s\n", server_get_client_address(self->server, socket));
}

void app_on_message(app_t *self, libwebsocket *socket, void *data, size_t len) {
    return ;
}

