CC = g++

linux: 
	$(CC) main.cpp -o main -lGL -lraylib -lm
