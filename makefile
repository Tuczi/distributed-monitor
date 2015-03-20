CXX=/usr/lib64/openmpi/bin/mpic++
CXXFLAGS=-g -Wall -Wextra --std=c++11
LFLAGS=

ODIR=obj
SDIR=src

_OBJS=main.o program-monitor.o distributed-mutex.o

OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))

main: $(OBJS)
	$(CXX) $(LFLAGS) $^ -o $@

$(OBJS): $(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
