#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <assert.h>  
#include <zmq.h>

int main(int argc, char** argv){  
    void* context = zmq_ctx_new();  
    void* publisher = zmq_socket(context, ZMQ_PUB);  
    assert(zmq_connect(publisher, "tcp://localhost:2019") == 0);  
  
    srand(0);
    const char* filter = (argc > 1)? argv[1] : "xxx";  
    printf("Start to publish, filter=%s\n", filter);  
  
    while(1){  
        char buf[64];  
        snprintf(buf, sizeof(buf), "%s ,i=%d j=%d", filter, rand(), rand()); 
        printf("Publish message: %s\n", buf);  
  
        zmq_msg_t msg;  
        zmq_msg_init_size(&msg, strlen(buf)+1);  
        memcpy(zmq_msg_data(&msg), buf, strlen(buf)+1);  
        sleep(2);
        zmq_msg_send(&msg, publisher, 0);  
        zmq_msg_close(&msg);  
  
        //usleep(rand() % 100 * 1000);  
    }  
  
    zmq_close(publisher);  
    zmq_ctx_destroy(context);  
    return 0;  
}  