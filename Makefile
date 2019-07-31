LIBS= -lvulkan -lglfw -lassimp 
CC=g++ -std=c++17 -I./include
BIN=a.out
SOURCES=$(wildcard src/*.cpp)
OBJS=$(patsubst src/%.cpp,obj/%.o,$(SOURCES))
SPARKOBJS=$(wildcard ../Vulkan-wrapper/obj/*.o)

all: $(BIN)

clean:
	rm obj/* && make all

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(SPARKOBJS) $(LIBS)

obj/main.o: src/main.cpp include/Application.hpp
	$(CC) -c $< -o $@ -g

obj/Camera.o: src/Camera.cpp include/Camera.hpp
	$(CC) -c $< -o $@ -g

obj/Mesh.o: src/Mesh.cpp include/Mesh.hpp
	$(CC) -c $< -o $@ -g

obj/BasicMesh.o: src/BasicMesh.cpp include/BasicMesh.hpp include/Mesh.hpp
	$(CC) -c $< -o $@ -g

obj/NotTexturedMesh.o: src/NotTexturedMesh.cpp include/NotTexturedMesh.hpp include/Mesh.hpp
	$(CC) -c $< -o $@ -g

obj/Application.o: src/Application.cpp \
	include/Application.hpp \
	include/Mesh.hpp \
	include/TextureHolder.hpp \
	include/Camera.hpp
	$(CC) -c $< -o $@ -g

obj/TextureHolder.o: src/TextureHolder.cpp \
	include/TextureHolder.hpp
	$(CC) -c $< -o $@ -g

obj/VertexClasses.o: src/VertexClasses.cpp \
	include/VertexClasses.hpp
	$(CC) -c $< -o $@ -g