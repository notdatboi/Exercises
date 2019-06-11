LIBS= -lvulkan -lglfw -lassimp 
CC=g++ -std=c++17
BIN=a.out
SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,obj/%.o,$(SOURCES))
SPARKOBJS=$(wildcard ../Spark/obj/*.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(SPARKOBJS) $(LIBS)

obj/main.o: main.cpp App.hpp
	$(CC) -c $< -o $@ -g

obj/App.o: App.cpp \
	App.hpp 
	$(CC) -c $< -o $@ -g