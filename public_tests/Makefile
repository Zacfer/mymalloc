CFLAGS=-pthread -std=c99 -D SEED=42 -D SIZE=65536 -D OPS=200 -D THREADS=10
CC=gcc


objects = mymalloc.o harness.o 
objects_mt = mymalloc_threadsafe.o harness_mt.o 

all: runtests runthreadedtests

runtests: $(objects)
	gcc $(CFLAGS) -o runtests $(objects)
	
runthreadedtests: $(objects_mt)
	gcc $(CFLAGS) -o runthreadedtests $(objects_mt)
	
clean: 
	rm runtests runthreadedtests $(objects) $(objects_mt) 2>/dev/null
	