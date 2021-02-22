COMP = mpicxx -Wall -std=c++17 -Ofast -o
OBJ = mpicxx -Wall -std=c++17 -Ofast -c

defaults: main

main: main.o utils.o
	$(COMP) main main.o utils.o

singlethread: single_thread.cpp
	g++ -Wall -std=c++17 -Ofast -g -o singlethread single_thread.cpp

%.o: %.cpp
	$(OBJ) $<

main.o: main.cpp utils.h

utils.o: utils.h utils.cpp

clean:
	-rm *.o $(objects)
	-rm main
	-rm singlethread