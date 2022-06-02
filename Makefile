SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.cpp=.hpp)
FLAGS=-DDEBUG -g -pthread

main: $(SOURCES) $(HEADERS)
	mpic++ $(SOURCES) $(FLAGS) -o bin/main

clear: clean

clean:
	rm main

run: main
	mpirun -mca btl tcp -mca pml ^ucx -hostfile bin/hostfile -oversubscribe -np 8 ./bin/main

local: main
	mpirun -np 8 ./bin/main
