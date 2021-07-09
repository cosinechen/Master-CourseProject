#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <assert.h>  
#include <zmq.h>  

int main(int argc, char** argv){  
    void* context = zmq_ctx_new(); 

    void* xsub = zmq_socket(context, ZMQ_XSUB);  
    assert(zmq_bind(xsub, "tcp://*:2019") == 0);
    void* xpub = zmq_socket(context, ZMQ_XPUB);  
    assert(zmq_bind(xpub, "tcp://*:2020") == 0);  
    
    zmq_setsockopt(xsub, ZMQ_SUBSCRIBE, "", 0);	

    zmq_proxy(xsub, xpub, NULL);  
  
    zmq_close(xsub);  
    zmq_close(xpub);  
    zmq_ctx_destroy(context);  
    return 0;  
}  