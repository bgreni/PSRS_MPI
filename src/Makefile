COMP = mpicxx -Wall -std=c++17 -g -Ofast -o
OBJ = mpicxx -Wall -std=c++17 -g -Ofast -c

defaults: main

main: main.o utils.o
	$(COMP) main main.o utils.o


%.o: %.cpp
	$(OBJ) $<

main.o: main.cpp utils.h

utils.o: utils.h utils.cpp

clean:
	-rm *.o $(objects)
	-rm main