CXX             := g++
DEBUG_LEVEL     := -g
EXTRA_CCFLAGS   := -Wall -Werror=return-type -Wextra -pedantic
CXXFLAGS        := $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)
LDFLAGS         := -lm -lstdc++ -pthread

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
#	rm *.o

ProCon: ProCon.o
	$(CXX) -o $@ $^

ProCon.o: $(PROC)/ProCon.cpp
	$(CXX) $(CXXFLAGS) -c $<

MH: MH.o
	$(CXX) -o $@ $^

MH.o: $(PROC)/MH.cpp
	$(CXX) $(CXXFLAGS) -c $<

Airline: Airline.o
	$(CXX) -o $@ $^

Airline.o: $(PROC)/Airline.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean

clean:
	rm -f *.o *.out $(ALL)
