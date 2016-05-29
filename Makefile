CFLAGS:=-Wall
CFILES:=$(wildcard src/*.cpp)
OBJS:=$(patsubst src/%.cpp,objs/%.o,$(CFILES))
HEADERS:=$(wildcard src/*.h)
INCLUDES:=portaudio/include
LIBS:=$(wildcard $(OS)/*.a)
LINK_FLAGS:=/usr/local/Cellar/portaudio/19.20140130/lib/libportaudio.a

OS:=$(shell uname)
ifeq ($(OS),Darwin)
	LINK_FLAGS:=$(LINK_FLAGS) -framework CoreMIDI -framework CoreAudio -framework AudioToolbox -framework CoreFoundation -framework AudioUnit -framework CoreServices -stdlib=libc++ -lfftw3
endif

all: ChromaTest

ChromaTest: $(HEADERS) $(OBJS) $(LIBS)
	g++ $(LINK_FLAGS) -L/usr/local/Cellar/fftw/3.3.4_1/lib $(CFLAGS) -o $@ $(OBJS) $(LIBS)

objs:
	-rm -rf objs
	mkdir objs

objs/%.o: src/%.cpp $(HEADERS) objs
	g++ $(CFLAGS) -I $(INCLUDES) -I/usr/local/Cellar/fftw/3.3.4_1/include -D__MACOSX_CORE__ -c -o $@ $<

clean:
	-rm -f ChromaTest
	-rm -rf objs
