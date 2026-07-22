main: main.c
	clang -lm -fsanitize=address main.c -o main

tests: tests.c
	clang -lm -fsanitize=address tests.c -o tests
