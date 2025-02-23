# Makefile for EE 450 Final Project
# compile options to be used in all compile commands
#     NOTE: $(CXXFLAGS) in the compile commands below will be replaced by the options after the =
CXXFLAGS = -Wall -ggdb -std=c++11
SERVERFLAGS = -Wall -ggdb -std=c++11 -pthread

all: client serverM serverS serverD serverU

client: client.cpp
	g++ $(SERVERFLAGS) -o client client.cpp

serverM: file_reader.o udp_connect.o serverM.cpp
	g++ $(SERVERFLAGS) -o serverM file_reader.o udp_connect.o serverM.cpp 

serverS: file_reader.o udp_connect.o serverS.cpp
	g++ $(SERVERFLAGS) -o serverS file_reader.o udp_connect.o serverS.cpp  

serverD: file_reader.o udp_connect.o serverD.cpp
	g++ $(SERVERFLAGS) -o serverD file_reader.o udp_connect.o serverD.cpp  

serverU: file_reader.o udp_connect.o serverU.cpp
	g++ $(SERVERFLAGS) -o serverU file_reader.o udp_connect.o serverU.cpp 

file_reader.o: file_reader.cpp file_reader.h
	g++ $(CXXFLAGS) -c file_reader.cpp

udp_connect.o: udp_connect.cpp udp_connect.h
	g++ $(CXXFLAGS) -c udp_connect.cpp

clean:
	-rm -f client serverM serverS serverD serverU file_reader.o udp_connect.o