LIBS= -lvulkan -lglfw -lassimp 
CC=g++ -std=c++17
BIN=a.out
SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,obj/%.o,$(SOURCES))
SPARKOBJS=$(wildcard ../Vulkan-wrapper/obj/*.o)

all: $(BIN)

clean:
	rm obj/* && make all

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(SPARKOBJS) $(LIBS)

obj/main.o: main.cpp Application.hpp
	$(CC) -c $< -o $@ -g

obj/Camera.o: Camera.cpp Camera.hpp
	$(CC) -c $< -o $@ -g

obj/Mesh.o: Mesh.cpp Mesh.hpp
	$(CC) -c $< -o $@ -g

obj/Application.o: Application.cpp \
	Application.hpp \
	Mesh.hpp \
	TextureHolder.hpp \
	Camera.hpp
	$(CC) -c $< -o $@ -g

obj/TextureHolder.o: TextureHolder.cpp \
	TextureHolder.hpp
	$(CC) -c $< -o $@ -g