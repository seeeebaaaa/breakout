all: breakout
CC = g++
CXXFLAGS = -std=c++20 -g -O0
CPPFLAGS = $(shell pkg-config --cflags gtk+-3.0) $(shell pkg-config --cflags freetype2)
LDFLAGS  = $(shell pkg-config --libs gtk+-3.0) $(shell pkg-config --libs freetype2) 

# for miniaudio
ifeq ($(shell uname -s),Linux)
	LDFLAGS += -lpthread -lm -ldl
endif
# prbly can also work without this but is safer, since it "assures" that apple trusts this program
ifeq ($(shell uname -s),Darwin) 
	LDFLAGS+= -framework CoreFoundation -framework CoreAudio -framework AudioToolbox
endif

SOURCE = main.cpp gui.cpp breakout.cpp util.cpp power_ups.cpp textures.cpp miniaudio.c sound.cpp bossfight.cpp
HEADER = gui.h breakout.h breakout_config.h util.h power_ups.hpp textures.hpp miniaudio_extra.h sound.hpp bossfight.hpp
OBJECT = $(SOURCE:%.cpp=%.o)

%.o: %.cpp $(HEADER)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

breakout: $(OBJECT)
	$(CC) $(CXXFLAGS) $^ $(LDFLAGS)  -o $@

dist:
	-mkdir breakout1_ada
	cp $(SOURCE) $(HEADER) Makefile breakout1_ada/
	unifdef -x2 -m -U REF breakout1_ada/breakout.cpp
	tar cfz breakout1_ada.tar.gz breakout1_ada/
	
# counts the number of lines in this project
ALL_FILES =  main.cpp breakout.cpp util.cpp power_ups.cpp textures.cpp sound.cpp bossfight.cpp breakout.h breakout_config.h util.h power_ups.hpp textures.hpp sound.hpp bossfight.hpp
lines: $(ALL_FILES)
	cat $(ALL_FILES) | wc -l

clean:
	rm -rf breakout1
