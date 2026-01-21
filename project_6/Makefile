CXX = clang++
CXXFLAGS = -std=c++23 -O2 -Wall -Wextra -fprebuilt-module-path=. -Wno-experimental-header-units -Wno-pragma-system-header-outside-header -Wno-eager-load-cxx-named-modules

# Used libraries
HEADERS = iostream vector utility cstddef unordered_map regex string memory ostream exception

# Modules written in the order required by compilation
MODULES = TextBase MothBase Text Moth Moths Colony

SYS_PCMS = $(addsuffix .pcm, $(HEADERS))
CLASS_PCMS = $(addsuffix .pcm, $(MODULES))
CLASS_OBJS = $(addsuffix .o, $(MODULES))

BUILD_PCM = $(CXX) $(CXXFLAGS) --precompile $< -o $@ $(addprefix -fmodule-file=, $(filter %.pcm, $^))

all: $(SYS_PCMS) $(CLASS_PCMS) $(CLASS_OBJS) main.o text_moths

$(SYS_PCMS): %.pcm :
	$(CXX) $(CXXFLAGS) -xc++-system-header -fmodule-header=system --precompile $* -o $@

%.o: %.pcm
	$(CXX) $(CXXFLAGS) -c $< -o $@

TextBase.pcm: TextBase.cppm cstddef.pcm
	$(BUILD_PCM)

MothBase.pcm: MothBase.cppm cstddef.pcm ostream.pcm
	$(BUILD_PCM)

Text.pcm: Text.cppm TextBase.pcm cstddef.pcm vector.pcm memory.pcm string.pcm utility.pcm iostream.pcm
	$(BUILD_PCM)

Moth.pcm: Moth.cppm MothBase.pcm
	$(BUILD_PCM)

Moths.pcm: Moths.cppm cstddef.pcm
	$(BUILD_PCM)

Colony.pcm: Colony.cppm unordered_map.pcm
	$(BUILD_PCM)

# Main 
main.o: main.cpp regex.pcm iostream.pcm memory.pcm exception.pcm string.pcm
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(addprefix -fmodule-file=, $(filter %.pcm, $^))

text_moths: main.o $(CLASS_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	
clean:
	rm -f *pcm *.o text_moths

.PHONY: all clean