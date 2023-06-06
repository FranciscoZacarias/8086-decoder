#include "sim86.h"

void decode(Memory* memory, u32 byte_count, SegmentedAccess start_position) {
	
	SegmentedAccess position = start_position;
	u32 bytes_left           = byte_count;
	
	while(bytes_left) {
		
	}
}

int main(int argc, char** argv) {	
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
	
	Memory* memory = (Memory*)malloc(sizeof(memory));
	FILE* output_file = stdout;
	
	for(int i = 1; i < argc; i++) {
		u32 bytes_read = load_file(memory, argv[i]);		
		decode(memory, output_file, bytes_read);
	}
	
	return 0;
}