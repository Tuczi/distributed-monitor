CXX=mpic++
CXXFLAGS=-g -Wall -Wextra --std=c++11 -O3
CXXFLAGS_SO= -fPIC
LFLAGS=

ODIR=obj
SDIR=src

_OBJS=process_monitor.o distributed_mutex.o mpi_proxy.o

OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))


LFLAGS_T= -Llib -ldist_monitor
TSDIR=test

_TOBJS=main.o

TOBJS=$(patsubst %,$(ODIR)/%,$(_TOBJS))

test_p: $(TOBJS)
	$(CXX) $(LFLAGS_T) $^ -o $@

lib/libdist_monitor.so: $(OBJS)
	$(CXX) -shared $(LFLAGS) $^ -o $@

$(OBJS): $(ODIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS_SO) $(CXXFLAGS) $< -o $@
	
$(TOBJS): $(ODIR)/%.o: $(TSDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@	

.PHONY: clean lib

clean:
	rm -f $(ODIR)/*.o
	rm -f lib/*.so

lib: clean lib/libdist_monitor.so
all: lib test_p
