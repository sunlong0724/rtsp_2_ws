#ifndef SERVER_H
#define SERVER_H

#include <libwebsockets.h>

#define libwebsocket                        lws                     
#define libwebsocket_context                lws_context             
#define libwebsocket_callback_reasons       lws_callback_reasons    
#define libwebsocket_protocols              lws_protocols           
#define libwebsocket_create_context         lws_create_context      
#define libwebsocket_context_destroy        lws_context_destroy
#define libwebsocket_get_socket_fd          lws_get_socket_fd
#define libwebsocket_callback_on_writable_all_protocol lws_callback_on_writable_all_protocol
#define libwebsocket_service                lws_service
#define libwebsocket_write                  lws_write
#define libwebsocket_context_user           lws_context_user
#define libwebsocket_write_protocol         lws_write_protocol
#define libwebsockets_get_peer_addresses    lws_get_peer_addresses




typedef struct server_client_t {
	libwebsocket *socket;
	struct server_client_t *next;
} client_t;

client_t *client_insert(client_t **head, libwebsocket *socket);
void client_remove(client_t **head, libwebsocket *socket);
#define client_foreach(HEAD, CLIENT) for(client_t *CLIENT = HEAD; CLIENT; CLIENT = CLIENT->next)

typedef enum {
	server_type_text = LWS_WRITE_TEXT,
	server_type_binary = LWS_WRITE_BINARY
} server_data_type_t;

typedef struct server_t {
	libwebsocket_context *context;
	size_t buffer_size;
	unsigned char *send_buffer_with_padding;
	unsigned char *send_buffer;
	void *user;
	
	int port;
	server_client_t *clients;
	
	typedef void (*on_connect_cb)(server_t *server, libwebsocket *wsi);
	typedef void (*on_message_cb)(server_t *server, libwebsocket *wsi, void *in, size_t len);
	typedef void (*on_close_cb)(server_t *server, libwebsocket *wsi);

    on_connect_cb on_connect;
    on_message_cb on_message;
    on_close_cb   on_close;
} server_t;

server_t *server_create(int port, size_t buffer_size, server_t::on_connect_cb ,  server_t::on_message_cb,
                        server_t::on_close_cb);
void server_destroy(server_t *self);
char *server_get_host_address(server_t *self);
char *server_get_client_address(server_t *self, libwebsocket *wsi);
void server_update(server_t *self);
void server_send(server_t *self, libwebsocket *socket, void *data, size_t size, server_data_type_t type);
void server_broadcast(server_t *self, void *data, size_t size, server_data_type_t type);





#endif
