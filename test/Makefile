all: test_server

test_server: test_server.o
	g++ $^ -o $@

clean:
	rm -rf *.o test_server
