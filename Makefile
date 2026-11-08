CFLAGS = -Wno-implicit-function-declaration 

all: main.c banner.c banner.h
	gcc $(CFLAGS) banner.c main.c -o final
	gcc -static $(CFLAGS) banner.c main.c -o final_static
	
static: main.c banner.c banner.h 
	gcc -static $(CFLAGS) banner.c main.c -o final_static

dynamic: main.c banner.c banner.h
	gcc $(CFLAGS) banner.c main.c -o final

clean:
	rm final final_static