CXX = g++
FLAGS += -std=c++11 -pthread -Wall -g

all: tracker client

# Objects

debug.o: debug.h
	$(CXX) $(FLAGS) -c debug.h

tcp.o: tcp.h tcp.cpp
	$(CXX) $(FLAGS) -c tcp.cpp

file.o: file.h file.cpp
	$(CXX) $(FLAGS) -c file.cpp

func.o: func.h func.cpp
	$(CXX) $(FLAGS) -c func.cpp

fileServer.o: fileServer.h fileServer.cpp
	$(CXX) $(FLAGS) -c fileServer.cpp

trackerServer.o: trackerServer.h trackerServer.cpp
	$(CXX) $(FLAGS) -c trackerServer.cpp

# Executables

tracker: tcp.o file.o func.o trackerServer.o
	$(CXX) $(FLAGS) -o tracker tcp.o file.o func.o trackerServer.o

client: tcp.o file.o func.o fileServer.o
	$(CXX) $(FLAGS) -o client tcp.o file.o func.o fileServer.o

clean:
	$(RM) core *.o tracker client
