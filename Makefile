queue:
	gcc -pthread -o main queue.c fuzzer.c

val:
	valgrind --leak-check=full --show-leak-kinds=all -v ./main

clean:
	rm -rf main