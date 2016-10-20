test: test.o liballocator.so
	gcc -o test test.o -L. -lallocator
	
test.o: test.c
	gcc -c test.c

liballocator.so: allocator.o
	gcc -shared -o liballocator.so allocator.o
	
allocator.o: allocator.c
	gcc -c -fpic allocator.c
	
clean:
	rm  allocator.o test.o liballocator.so
