CXX = g++
FLAGS += -std=c++11 -pthread -Wall -g

all: tracker client

# Objects

debug.o: debug.h
	$(CXX) $(FLAGS) -c debug.h

Client.o: Client.h Client.cpp
	$(CXX) $(FLAGS) -c Client.cpp

tcp.o: tcp.h tcp.cpp
	$(CXX) $(FLAGS) -c tcp.cpp

file.o: file.h file.cpp
	$(CXX) $(FLAGS) -c file.cpp

func.o: func.h func.cpp
	$(CXX) $(FLAGS) -c func.cpp

hash.o: hash.h hash.cpp
	$(CXX) $(FLAGS) -c hash.cpp

fileServer.o: fileServer.cpp
	$(CXX) $(FLAGS) -c fileServer.cpp

trackerServer.o: trackerServer.cpp
	$(CXX) $(FLAGS) -c trackerServer.cpp

# Executables

tracker: Client.o tcp.o file.o func.o hash.o debug.o trackerServer.o
	$(CXX) $(FLAGS) -o tracker Client.o tcp.o file.o func.o trackerServer.o

client: Client.o tcp.o file.o func.o hash.o debug.o fileServer.o
	$(CXX) $(FLAGS) -o client Client.o tcp.o file.o func.o fileServer.o

clean:
	$(RM) core *.o tracker client
