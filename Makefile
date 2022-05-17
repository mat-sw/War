SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.cpp=.hpp)
FLAGS=-DDEBUG -g

main: $(SOURCES) $(HEADERS)
	mpic++ $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main

run: main
	mpirun -mca btl tcp -mca pml ^ucx -hostfile hostfile -oversubscribe -np 8 ./main
