CXX             := g++
DEBUG_LEVEL     := -g
EXTRA_CCFLAGS   := -Wall -Werror=return-type -Wextra -pedantic -Wno-unused-parameter
OPT             := -O0
CXXFLAGS        := $(DEBUG_LEVEL) $(EXTRA_CCFLAGS) $(OPT)
LDLIBS          := -lm -lstdc++ -pthread

INC             := include
SRC             := src
PROC            := proc
TEST            := tests
SHEL            := ProCon.o mh.o Airline.o
ALL             := ProCon MH Airline

# $@ targt name
# $< first prerequisite
# $^ all prerequisites

all: $(ALL)
	rm *.o

ProCon: ProCon.o
	$(CXX) -o $@ $^ $(LDLIBS)

ProCon.o: $(PROC)/ProCon.cpp
	$(CXX) $(CXXFLAGS) -c $<

MH: MH.o
	$(CXX) -o $@ $^ $(LDLIBS)

MH.o: $(PROC)/MH.cpp
	$(CXX) $(CXXFLAGS) -c $<

Airline: Airline.o
	$(CXX) -o $@ $^ $(LDLIBS)

Airline.o: $(PROC)/Airline.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean

clean:
	rm -f *.o *.out $(ALL)
