#include "sim86.h"

#include "Memory.h"
#include "Text.h"
#include "Decoder.h"


#include "Memory.cpp"
#include "Text.cpp"
#include "Decoder.cpp"

void run_decoder(Memory* memory, u32 byteCount, SegmentedAccess startPosition) {
	SegmentedAccess position = startPosition;
	u32 bytesLeft            = byteCount;
	
	while(bytesLeft) {
		Instruction instruction = decode_instruction(memory, &position);
		
		if (!instruction.operation) {
			fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
			break;
		}
			
		if (bytesLeft < instruction.size) {
			fprintf(stderr, "ERROR: Instruction extends outside disassembly region.\n");
			break;
		} 
		
		bytesLeft -= instruction.size;
		print_instruction(instruction, stdout);
		printf("\n");
	}
}

int main(int argc, char** argv) {	
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
	
	Memory* memory = (Memory*)malloc(sizeof(Memory));
	
	for(int i = 1; i < argc; i++) {
		u32 bytesRead = load_file_to_memory(memory, argv[i], 0);		

		printf("\n; %s disassembly:\n", argv[i]);
		printf("bits 16\n");
		run_decoder(memory, bytesRead, { 0, 0 });
	}
	
	free(memory);
	return 0;
}