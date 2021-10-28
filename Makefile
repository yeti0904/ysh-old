src   = src/main.cc src/scripting.cc
hdr   = src/scripting.hh
out   = bin/ysh
std   = c++17
flags = -s -lreadline -lstdc++

build: $(src) $(hdr)
	mkdir -p bin
	$(CC) $(src) $(flags) -o $(out) -std=$(std)