C++ = g++

amenability_test:
	$(C++) amenability_test.cpp src/Graph.cpp src/ColorRefinementAmenability.cpp src/AuxiliaryMethods.cpp -Isrc -std=c++11 -O2 -o amenability_test

amenability_test:	amenability_test.cpp
amenability_test:	src/Graph.cpp src/Graph.h
amenability_test:	src/ColorRefinementAmenability.cpp src/ColorRefinementAmenability.h
amenability_test:	src/AuxiliaryMethods.cpp src/AuxiliaryMethods.h

