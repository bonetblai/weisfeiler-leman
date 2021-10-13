C++ = clang++ -I/opt/homebrew/include

amenability_test:
	$(C++) amenability_test.cpp src/ColorRefinementAmenability.cpp src/AuxiliaryMethods.cpp -Isrc -std=c++11 -O2 -o amenability_test

wl:
	$(C++) wl.cpp src/ColorRefinementAmenability.cpp src/AuxiliaryMethods.cpp -Isrc -std=c++11 -O2 -o wl

amenability_test:	amenability_test.cpp
amenability_test:	src/Graph.h
amenability_test:	src/ColorRefinementAmenability.cpp src/ColorRefinementAmenability.h
amenability_test:	src/AuxiliaryMethods.cpp src/AuxiliaryMethods.h

wl:			wl.cpp
wl:			src/Graph.h
wl:			src/ColorRefinementAmenability.cpp src/ColorRefinementAmenability.h
wl:			src/AuxiliaryMethods.cpp src/AuxiliaryMethods.h
