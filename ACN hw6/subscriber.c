#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <assert.h>  
#include <zmq.h>  

int main(int argc, char** argv){  
    void* context = zmq_ctx_new();  
    void* subscriber = zmq_socket(context, ZMQ_SUB);  
    assert(zmq_connect(subscriber, "tcp://localhost:2020") == 0);
  
    for(int i=1; i<argc; i++){
        const char* filter = (argc > 1)? argv[i] : "xxx"; 
        assert(zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter)) == 0);  
        printf("Set filter to %s\n", filter); 
    }  

    while(1){  
        zmq_msg_t msg;  
        zmq_msg_init(&msg);  
        zmq_msg_recv(&msg, subscriber, 0);  
        printf("Get a message: %s\n", zmq_msg_data(&msg));  
        zmq_msg_close(&msg);  
    }  
  
    zmq_close(subscriber);  
    zmq_ctx_destroy(context);  
  
    return 0;  
}  