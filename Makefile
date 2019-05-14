EXE = ACT

OBJS_EXE = HSLAPixel.o lodepng.o PNG.o main.o toqutree.o stats.o
OBJS_EXET = HSLAPixel.o lodepng.o PNG.o toqutree.o stats.o

CXX = clang++
CXXFLAGS = -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD = clang++
LDFLAGS = -std=c++1y -stdlib=libc++ -lpthread -lm

all : ACT

$(EXE) : $(OBJS_EXE)
	$(LD) $(OBJS_EXE) $(LDFLAGS) -o $(EXE)

#object files
HSLAPixel.o : util/HSLAPixel.cpp util/HSLAPixel.h
	$(CXX) $(CXXFLAGS) util/HSLAPixel.cpp -o $@

PNG.o : util/PNG.cpp util/PNG.h util/HSLAPixel.h util/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) util/PNG.cpp -o $@

lodepng.o : util/lodepng/lodepng.cpp util/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) util/lodepng/lodepng.cpp -o $@

stats.o : stats.h stats.cpp util/HSLAPixel.h util/PNG.h
	$(CXX) $(CXXFLAGS) stats.cpp -o $@

toqutree.o : toqutree.h toqutree.cpp stats.h util/PNG.h util/HSLAPixel.h
	$(CXX) $(CXXFLAGS) toqutree.cpp -o $@

testComp.o : testComp.cpp util/PNG.h util/HSLAPixel.h toqutree.h
	$(CXX) $(CXXFLAGS) testComp.cpp -o testComp.o

main.o : main.cpp util/PNG.h util/HSLAPixel.h toqutree.h
	$(CXX) $(CXXFLAGS) main.cpp -o main.o

clean :
	-rm -f *.o $(EXE)
