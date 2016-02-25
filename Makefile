CPP      = g++

SOURCES = board.cpp pathfinder.cpp
HEADERS = $(SOURCES:.cpp=.hpp)
OBJECTS = $(SOURCES:.cpp=.o)

GTK_SOURCES = board_display.cpp
GTK_HEADERS = $(GTK_SOURCES:.cpp=.hpp)
GTK_OBJECTS = $(GTK_SOURCES:.cpp=.o)

ALL_OBJECTS = $(OBJECTS) $(GTK_OBJECTS)
ALL_HEADERS = $(HEADERS) $(GTK_HEADERS)

CPPFLAGS  += -g -Wall -std=c++11 -pedantic
LDFLAGS +=

GTK_CPPFLAGS  = $(CPPFLAGS) $(shell pkg-config --cflags gtk+-3.0 librsvg-2.0)
GTK_LDFLAGS = $(LDFLAGS) $(shell pkg-config --libs gtk+-3.0 librsvg-2.0)

all : example showboard editor

$(OBJECTS) : %.o : %.cpp
	$(CPP) -c $(CPPFLAGS) $? -o $@

$(GTK_OBJECTS) : %.o : %.cpp
	$(CPP) -c $(GTK_CPPFLAGS) $? -o $@

########## example ##########

EXAMPLE_SOURCES = example.cpp
EXAMPLE_OBJECTS = $(EXAMPLE_SOURCES:.cpp=.o)

example : $(EXAMPLE_OBJECTS) $(OBJECTS) $(HEADERS)
	$(CPP) $^ -o $@ $(LDFLAGS)
	
$(EXAMPLE_OBJECTS): %.o : %.cpp
	$(CPP) -c $(CPPFLAGS) $? -o $@

########## showboard ##########

SHOWBOARD_SOURCES = showboard.cpp
SHOWBOARD_OBJECTS = $(SHOWBOARD_SOURCES:.cpp=.o)

showboard : $(SHOWBOARD_OBJECTS) $(ALL_OBJECTS) $(ALL_HEADERS)
	$(CPP) $^ -o $@ $(GTK_LDFLAGS)

$(SHOWBOARD_OBJECTS): %.o : %.cpp
	$(CPP) -c $(GTK_CPPFLAGS) $? -o $@

########## editor ##########

EDITOR_SOURCES = editor.cpp
EDITOR_OBJECTS = $(EDITOR_SOURCES:.cpp=.o)

editor : $(EDITOR_OBJECTS) $(ALL_OBJECTS) $(ALL_HEADERS)
	$(CPP) $^ -o $@ $(GTK_LDFLAGS)

$(EDITOR_OBJECTS): %.o : %.cpp
	$(CPP) -c $(GTK_CPPFLAGS) $? -o $@

########## cleanup ##########

clean:
	@rm -f example showboard editor $(ALL_OBJECTS) $(EXAMPLE_OBJECTS) $(SHOWBOARD_OBJECTS) $(EDITOR_OBJECTS)
