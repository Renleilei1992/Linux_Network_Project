target:bin/client
objs:obj/view.o obj/client.o

obj/view.o:src/view.c
	gcc -c src/view.c -o obj/view.o -g -Wall -I include

obj/client.o:src/client.c
	gcc -c src/client.c -o obj/client.o -g -Wall -I include

bin/client:src/main.c obj/view.o obj/client.o
	gcc src/main.c obj/view.o obj/client.o -o bin/client -g -Wall -I include -lpthread
