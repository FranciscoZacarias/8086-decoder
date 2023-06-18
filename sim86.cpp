#include "sim86.h"

#include "Memory.h"
#include "Text.h"
#include "Decoder.h"
#include "Simulation.h"

#include "Memory.cpp"
#include "Text.cpp"
#include "Decoder.cpp"
#include "Simulation.cpp"

char const *registrs[] = {
	{""},
	{"AX"},
	{"BX"},
	{"CX"},
	{"DX"},
	{"SP"},
	{"BP"},
	{"SI"},
	{"DI"},
	{"Flags"},
};

b32 execute = 0;

Memory* memory;

SimRegister simulatedRegisters[Register_count];

void run_sim8086(Memory* memory, u32 byteCount, SegmentedAccess startPosition) {
	SegmentedAccess position = startPosition;
	u32 bytesLeft            = byteCount;
	
	printf("bits 16\n");
	
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
		if (execute) {
			simulate_instruction(simulatedRegisters, instruction, stdout);
		}
		
		printf("\n");
	}
	
	printf("Final Registers:");
	for(int i = 1; i < Register_count; i++) {
		if (simulatedRegisters[i].reg == Register_Flags) {
			printf("\tFlags: ");
			for(int j = 0; j < 8; j++) {
				printf("%c", (simulatedRegisters[i].data16 >> j) & 1 ? '1' : '0');
			}
			printf(" (%d)", simulatedRegisters[i].data16);
			continue;
		}
		printf("\t%s: 0x%04hx (%d)\n", registrs[i], simulatedRegisters[i].data16, simulatedRegisters[i].data16);
	}
	
	printf("\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
	
	if (argc > 2) {
		for(int i = 1; i < argc; i++) {
			if (argv[i][0] != '-')  continue;
			
			if (strcmp((const char*)argv[i], "-exec") == 0) {
				execute = 1;
			} else {
				fprintf(stderr, "ERROR: Unknown option %s\n", argv[2]);
				return 1;
			}
		}
	}
	
	for(int i = 0; i < Register_count; i++) {
		SimRegister sr = { (Register)i, 0 };
		simulatedRegisters[i] = sr;
	}
	
	memory = (Memory*)malloc(sizeof(Memory));
	
	for(int i = 1; i < argc; i++) {
		if (argv[i][0] == '-')  continue;
		
		printf("\n; %s disassembly:\n", argv[i]);
		
		u32 bytesRead = load_file_to_memory(memory, argv[i], 0);		

		run_sim8086(memory, bytesRead, { 0, 0 });
	}
	
	free(memory);
	
	return 0;
}