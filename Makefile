queue:
	gcc -o main queue.c fuzzer.c

val:
	valgrind --leak-check=full -v ./main
	
clean:
	rm -rf main