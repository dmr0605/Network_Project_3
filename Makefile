all:
	gcc -o ./server/server.c ./server/server -pthread
  
clean:
	rm -f ./server/server 