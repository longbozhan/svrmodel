all : server

server : main.o Utils.o svrkit.o processMaster.o connectionMaster.o parameter.o notify.o shmQueue.o sem.o 
	g++ -g $^ -o $@ -lpthread -levent

shmQueue.o : shmQueue.cpp 
	g++ -g -c $^ -o $@

clean:
	rm -rf *.o server
