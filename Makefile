CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++11

OBJECTS = main.o ping_socket.o

all: traceroute

traceroute: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o traceroute $(OBJECTS) 

clean:
	rm -f *.o

distclean: clean
	rm -f traceroute

.PHONY: all clean archive
