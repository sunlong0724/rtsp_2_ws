
#include "app.h"
#include <signal.h>

static bool g_running_flag = false;

void sig_cb(int sig_num){
    if (2 == sig_num){
        g_running_flag = false;
    }
}

int main(int argc, char** argv){

    g_running_flag = true;
    signal( 2, sig_cb );

    /*
    if (argc < 3){
        fprintf(stdout," usage: %s rtsp_url lws_port\n", basename ( argv[0] ));
        return 0;
    }
    */

   // app_create_and_start( argv[1], atoi( argv[2] ) );
    app_t* app = app_create_and_start( "rtsp://192.168.1.7:8554/slamtv10.264",  8082);

    while(g_running_flag){
        std::this_thread::sleep_for( std::chrono::milliseconds(10) );;
    }

    app_destroy( app );

    return 0;
}
