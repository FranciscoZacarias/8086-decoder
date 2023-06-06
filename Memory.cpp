#include "Memory.h"

u32 load_file(Memory* memory, char* filename, u32 offset) {
	u32 total_bytes = 0;
	
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Can't open the file: %s\n", filename);
		return 0;
	}
	
	total_bytes = fread(memory->bytes + offset, 1, ArrayCount(memory->bytes), fp);
	fclose(fp);
	
	return total_bytes;
}
