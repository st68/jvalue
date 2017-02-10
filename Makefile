
testJSON : testJSON.o jvalue.o
	g++ -o $@ testJSON.o jvalue.o

jvalue.o : jvalue.cpp jvalue.h
testJSON.o : testJSON.cpp


CFLAGS = \
	-std=gnu++11 \
	-Wall \
	-Wno-write-strings \
	-Wno-parentheses \
	-Wno-reorder \
	-Wno-address \
	-Werror \

.cpp.o :
	gcc $(CFLAGS) -c $<

