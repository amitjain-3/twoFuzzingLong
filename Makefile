fuzzer:
	gcc -pthread -o fuzzer queue.c fuzzer.c

all:
	gcc -pthread -o all queue.c fuzzer.c final_test_prog.c

test:
	gcc -o test final_test_prog.c

val:
	valgrind --leak-check=full --show-leak-kinds=all -v ./fuzzer

clean:
	rm -rf main