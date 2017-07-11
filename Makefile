CXX=g++
FLAGS=-std=c++11 -Wall -Wpedantic
OUTPUT=lcm_demo
FILES=main.cpp
#FILES=hp_batch.cpp

all: release

release: $(FILES)
	$(CXX) $(FLAGS) -O3 $(FILES) -o $(OUTPUT)
	
debug: $(FILES)
	$(CXX) $(FLAGS) -g $(FILES) -o $(OUTPUT)

clean:
	rm -rf *.o gmon.out $(OUTPUT)
