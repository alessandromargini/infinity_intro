PS5_PAYLOAD_SDK ?= /opt/ps5-payload-sdk

CXX := $(PS5_PAYLOAD_SDK)/bin/prospero-clang++
PKGCONFIG := $(PS5_PAYLOAD_SDK)/bin/prospero-pkg-config

CXXFLAGS := -std=c++17 -O2 \
  $(shell $(PKGCONFIG) --cflags sdl2 SDL2_image SDL2_mixer)

LDADD := $(shell $(PKGCONFIG) --libs --static sdl2 SDL2_image SDL2_mixer) -lSDL2main

ELF := infinity_intro.elf

all: $(ELF)

$(ELF): main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDADD)

clean:
	rm -f $(ELF)


PS5_PORT ?= 9021
test: $(ELF)
	nc -q0 $(PS5_HOST) $(PS5_PORT) < $(ELF)

.PHONY: all clean test
