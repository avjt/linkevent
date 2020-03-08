CC = gcc
CXX = g++ -std=c++17

default: linkevent linkcheck

linkevent: linkevent.c
	$(CC) -o $@ $<

linkcheck: linkcheck.cc
	$(CXX) -o $@ $< -lcurl -lpthread

clean:
	rm -rf linkevent linkcheck core core.*
