abac-test:
	g++ test.cpp bitstream.cpp cabac.cpp memory.cpp -O3 -o abac-test
debug:
	g++ test.cpp bitstream.cpp cabac.cpp memory.cpp -DDEBUG -Wall -g -o abac-test
abac-big-test:
	g++ big-test.cpp bitstream.cpp cabac.cpp memory.cpp -O3 -lz -o abac-big-test
big-debug:
	g++ big-test.cpp bitstream.cpp cabac.cpp memory.cpp -DDEBUG -Wall -g -lz -o abac-big-test
clean:
	rm -f abac-test abac-big-test
