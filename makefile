all:
	g++ -Wall serial.cpp -c
	g++ -Wall slip.cpp -c
	g++ -Wall nodo.cpp -c
	g++ -Wall slip.o serial.o nodo.o -o nodo
mk_socket:
	g++ -Wall virtualSocket.cpp -o virtualSocket
run_socket:
	./virtualSocket tmp/p1 tmp/p2 & ./virtualSocket tmp/p3 tmp/p4 & ./virtualSocket tmp/p5 tmp/p6
nodo1:
	./nodo 192.168.130.1 tmp/p1 tmp/p6
nodo2:
	./nodo 192.168.130.2 tmp/p3 tmp/p2
nodo3:
	./nodo 192.168.130.3 tmp/p5 tmp/p4
clean:
	rm *.o