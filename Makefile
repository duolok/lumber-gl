CXX = g++
CXXFLAGS = -std=c++11 -Wall -I/usr/include/freetype2
LDFLAGS = -lGLEW -lGL -lglfw -lGLU -lfreetype

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = lumber_gl

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXEC)
