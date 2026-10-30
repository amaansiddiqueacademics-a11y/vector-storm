# VECTOR STORM - GNU Makefile
#   Windows (MSYS2 / MinGW-w64):  make        -> game.exe (needs libfreeglut.dll next to it)
#   Linux (freeglut3-dev):        make        -> game     (runs silently: sound is Windows-only)
#   make run / make clean

CXX ?= g++
CXXFLAGS ?= -O2 -Wall -std=c++11
override CXXFLAGS += -Iinclude   # always needed, even if CXXFLAGS is set on the command line

SRCS = main.cpp \
       src/Audio/SoundEngine.cpp \
       src/Graphics/Renderer.cpp \
       src/Graphics/ParticleSystem.cpp \
       src/Graphics/Starfield.cpp \
       src/Graphics/CGLab.cpp \
       src/Entities/Powerup.cpp \
       src/Core/SaveSystem.cpp

HEADERS = $(wildcard include/*/*.hpp)

ifeq ($(OS),Windows_NT)
    TARGET = game.exe
    # Static C++ runtime + winpthread: no libwinpthread-1.dll needed at run time
    LIBS = -static-libgcc -static-libstdc++ -lfreeglut -lopengl32 -lglu32 -lwinmm \
           -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
else
    TARGET = game
    LIBS = -lglut -lGLU -lGL -lpthread
endif

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
