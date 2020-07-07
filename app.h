#ifndef APP_H
#define APP_H

#include "demux_and_dec.h"
#include "encode_video.h"
#include "server.h"

#define APP_FRAME_BUFFER_SIZE (1024*1024)

typedef enum {
	jsmpeg_frame_type_video = 0xFA010000,
	jsmpeg_frame_type_audio = 0xFB010000
} jsmpeg_trame_type_t;

typedef struct _jsmpeg_frame_t{
	jsmpeg_trame_type_t type;
	int size;
	//char data[0];
    char*  data;
} jsmpeg_frame_t;

typedef struct {
    CDemuxAndDec *demux_and_dec;
	CEncodeVideo *encoder;
	server_t *server;
    int lws_port;
    jsmpeg_frame_t *frame;
} app_t;


app_t *app_create_and_start(const char* rtsp_url, int lws_port);
void app_destroy(app_t *self);
void app_run(app_t *self);

void app_on_connect(app_t *self, libwebsocket *socket);
void app_on_close(app_t *self, libwebsocket *socket);
void app_on_message(app_t *self, libwebsocket *socket, void *data, size_t len);

#endif
