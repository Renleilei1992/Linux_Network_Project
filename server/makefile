target:bin/server
objs:obj/saveList.o

obj/saveList.o:src/saveList.c
	gcc -c src/saveList.c -o obj/saveList.o -g -Wall -I include

bin/server:src/server.c obj/saveList.o
	gcc src/server.c obj/saveList.o -o bin/server -g -Wall -I include -lpthread
