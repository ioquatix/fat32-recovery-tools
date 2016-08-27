
#include <iostream>
#include <fstream>
#include <utility>

const unsigned long BufferLength = 1024 * 1024 * 4;

struct Buffer {
	char *data;
	unsigned long length;
	
	Buffer(unsigned long _length) : length(_length) {
		data = new char[length];
	}
	
	~Buffer () {
		delete[] data;
	}
};

int main (int argc, char** argv) {
	Buffer buffer(BufferLength);
	
	if (argc < 3) {
		std::cout << argv[0] << " {input file} {search token} [start offset]" << std::endl;
		exit(-1);
	}
	
	std::string token (argv[2]);
	
	if (token.size() == 0) {
		std::cout << "Token length must be > 0" << std::endl;
		exit(-2);
	}
	
	char c;
	unsigned n = 0;
	unsigned long long p = 0;
	
	std::ifstream input;
	
	// Set a big input buffer
	input.rdbuf()->pubsetbuf(buffer.data, buffer.length);
	input.open(argv[1], std::ios::binary);
	
	if (argc == 4) {
		input.seekg(strtoull(argv[3], NULL, 10));
	}
	
	while (!input.eof()) {
		p += 1; // Position Counter
		input >> c;
		
		if (input.bad()) {
			std::cout << "Bad input at byte " << input.tellg() << std::endl;
			input.close();
			exit(-3);
		}
		
		if (c == token[n]) {
			n += 1;
			
			if (n == token.size()) {
				std::cout << "Token found at offset: " << (input.tellg() - (std::streamoff)token.size()) << std::endl;
				n = 0;
			}
		} else {
			n = 0;
		}
				
		if ((p % (1024 * 1024 * 100)) == 0)
			std::cout << "Read " << (input.tellg() / (1024 * 1024)) << "mb (" << input.tellg() << ")." << std::endl;
	}
	
	input.close();
}