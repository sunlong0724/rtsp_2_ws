#include <stdlib.h>
#include <stdio.h>
#include "server.h"

client_t *client_insert(client_t **head, libwebsocket *socket) {
	client_t *client = (server_client_t *)malloc(sizeof(server_client_t));
	client->socket = socket;
	client->next = *head;
	*head = client;
	return client;
}

void client_remove(client_t **head, libwebsocket *socket) {
	for( client_t **current = head; *current; current = &(*current)->next ) {
		if( (*current)->socket == socket ) {
			client_t* next = (*current)->next;
			free(*current);
			*current = next;
			break;
		}
	}
}

static int callback_websockets(
    struct libwebsocket *, enum libwebsocket_callback_reasons,void *, void *, size_t);

static struct libwebsocket_protocols server_protocols[2]{0};

server_t *server_create(int port, size_t buffer_size, 
                        server_t::on_connect_cb cb1 ,  server_t::on_message_cb cb2,
                        server_t::on_close_cb  cb3){

	server_t *self = (server_t *)malloc(sizeof(server_t));
	memset(self, 0, sizeof(server_t));

    self->on_connect = cb1;
    self->on_message = cb2;
    self->on_close = cb3;

	
	self->buffer_size = buffer_size;
	size_t full_buffer_size = LWS_SEND_BUFFER_PRE_PADDING + buffer_size + LWS_SEND_BUFFER_POST_PADDING;
	self->send_buffer_with_padding = (unsigned char *)malloc(full_buffer_size);
	self->send_buffer = &self->send_buffer_with_padding[LWS_SEND_BUFFER_PRE_PADDING];

	self->port = port;
	self->clients = NULL;

	lws_set_log_level(LLL_ERR | LLL_WARN, NULL);

	struct lws_context_creation_info info = {0};
	info.port = port;
	info.gid = -1;
	info.uid = -1;
	info.user = (void *)self;

    struct libwebsocket_protocols protocols[] = {
    	{ "ws", callback_websockets, sizeof(int), 1024*1024 },
    	{ NULL, NULL, 0 /* End of list */ }
    };

    server_protocols[0] = protocols[0];

	info.protocols = server_protocols;
	self->context = libwebsocket_create_context(&info);

	if( !self->context ) {
		server_destroy(self);
		return NULL;
	}


    fprintf(stdout,"%s %x %x\n", __FUNCTION__, self, self->on_connect);

	return self;
}

void server_destroy(server_t *self) {
	if( self == NULL ) { return; }

	if( self->context ) {
		libwebsocket_context_destroy(self->context);
	}
	
	free(self->send_buffer_with_padding);
	free(self);
}

char *server_get_host_address(server_t *self) {
	char host_name[80];
	hostent *host;
	if( 
		gethostname(host_name, sizeof(host_name)) == -1 ||
		!(host = gethostbyname(host_name))
	) {
		return "127.0.0.1";
	}

	return inet_ntoa( *(struct in_addr *)(host->h_addr_list[0]) );
}

char *server_get_client_address(server_t *self, libwebsocket *wsi) {
	static char ip_buffer[32];
	static char name_buffer[32];

	libwebsockets_get_peer_addresses(
		 wsi, libwebsocket_get_socket_fd(wsi), 
		name_buffer, sizeof(name_buffer), 
		ip_buffer, sizeof(ip_buffer)
	);
	return ip_buffer;
}

void server_update(server_t *self) {
	libwebsocket_callback_on_writable_all_protocol(self->context, &(server_protocols[0]));
	libwebsocket_service(self->context, 0);
}

void server_send(server_t *self, libwebsocket *socket, void *data, size_t size, server_data_type_t type) {
	// Caution, this may explode! The libwebsocket docs advise against ever calling libwebsocket_write()
	// outside of LWS_CALLBACK_SERVER_WRITEABLE. Honoring this advise would complicate things quite
	// a bit - and it seems to work just fine on my system as it is anyway.
	// This won't work reliably on mobile systems where network buffers are typically much smaller.
	// ¯\_(ツ)_/¯

	if( size > self->buffer_size ) {
		printf("Cant send %d bytes; exceeds buffer size (%d bytes)\n", size, self->buffer_size);
		return;
	}
	memcpy(self->send_buffer, data, size);
	libwebsocket_write(socket, self->send_buffer, size, (libwebsocket_write_protocol)type);
}

void server_broadcast(server_t *self, void *data, size_t size, server_data_type_t type) {
	if( size > self->buffer_size ) {
		printf("Cant send %d bytes; exceeds buffer size (%d bytes)\n", size, self->buffer_size);
		return;
	}
	memcpy(self->send_buffer, data, size);

    //fprintf(stdout,"1 %s \n", __FUNCTION__);

	client_foreach(self->clients, client) {
		libwebsocket_write(client->socket, self->send_buffer, size, (libwebsocket_write_protocol)type);
	}
    //fprintf(stdout,"2 %s %p\n", __FUNCTION__ ,self->context);
	libwebsocket_callback_on_writable_all_protocol(self->context,&(server_protocols[0]));

    //fprintf(stdout,"3 %s \n", __FUNCTION__);
}

static int callback_websockets(
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len
) {

	server_t *self = (server_t *)user;
    //server_t *self = (server_t *)(lws_wsi_user( wsi ));

	switch( reason ) {
		case LWS_CALLBACK_ESTABLISHED:
			client_insert(&self->clients, wsi);
            fprintf(stdout,"####%s %x %x\n", __FUNCTION__, self, self->on_connect);
			if( self->on_connect ) {
				self->on_connect(self, wsi);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			if( self->on_message ) {
				self->on_message(self, wsi, in, len);
			}
			break;

		case LWS_CALLBACK_CLOSED:
			client_remove(&self->clients, wsi);		
		
			if( self->on_close ) {
				self->on_close(self, wsi);
			}
			break;
	}
	
	return 0;
}

