all:
	gcc publisher.c -o publisher -lzmq
	gcc subscriber.c -o subscriber -lzmq
	gcc proxy.c -o proxy -lzmq
	
clean:
	rm publisher subscriber proxy
