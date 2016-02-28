CXXFLAGS += --std=c++11 -Wall -O2 -pedantic -g
LDFLAGS += -s

all:	convbdf convflashbmp

convbdf.o:	convbdf.c
convflashbmp.o:	convflashbmp.cpp lz.h
lz.o:	lz.c lz.h

convbdf:	convbdf.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

convflashbmp:	convflashbmp.o lz.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lIL -lILU


clean:
	rm -f *.o convbdf convflashbmp

