src   = src/main.cc
out   = bin/ysh
std   = c++17
flags = -s -lreadline -lstdc++

build: $(src)
	mkdir -p bin
	$(CC) $(src) $(flags) -o $(out) -std=$(std)