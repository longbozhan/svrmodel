all : server

server : main.o Utils.o svrkit.o processMaster.o connectionMaster.o parameter.o notify.o shmQueue.o sem.o 
	g++ -g $^ -o $@ -lpthread -levent

clean:
	rm -rf *.o server
