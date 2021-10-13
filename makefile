C++ = clang++ -I/opt/homebrew/include

wl:
	$(C++) wl.cpp src/ColorRefinement.cpp -Isrc -std=c++11 -O2 -o wl

wl:			wl.cpp
wl:			src/Graph.h
wl:			src/ColorRefinement.h
wl:			src/ColorRefinement.cpp

