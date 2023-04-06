#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

enum Operation_Code {
	INVALID = 0b00000000,
	
	// MOVs
	MOV_REGMEM_TO_REGMEM = 0b100010,
	MOV_IM_TO_REG        = 0b1011,
	
};

struct Instruction {
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
};

const char* byte_registers[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}; // W = 0, 8  bit registers
const char* word_registers[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}; // W = 1, 16 bit registers

Operation_Code get_operation_code(uint8_t byte) {
	// NOTE(fz): Op codes always start from the upper bits, so for example, 
	if (((byte & 0b11110000) >> 4) == MOV_IM_TO_REG) {
		return MOV_IM_TO_REG;
	}
	
	if (((byte & 0b11111100) >> 2) == MOV_REGMEM_TO_REGMEM) {
		return MOV_REGMEM_TO_REGMEM;
	}
	
	return INVALID;
}

int main(int argc, char** argv) {	
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
	
    FILE* fp = fopen(argv[1], "rb");
	if (fp == NULL) {
        printf("Error opening file %s.\n", argv[1]);
        return 1;
    }

	printf("bits 16\n");
	
    struct Instruction instruction = { 0, 0 ,0 ,0 };
    while (fread(&instruction.byte1, sizeof(instruction.byte1), 1, fp) == 1) {
		// TODO(fz): Clean this up. Make sure we keep the first byte, but we should clean the other ones.
		instruction.byte2 = 0;
		instruction.byte3 = 0;	
		instruction.byte4 = 0;
		
		Operation_Code opcode = get_operation_code(instruction.byte1);
		
		switch(opcode) {
			case MOV_IM_TO_REG: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				// W Byte
				uint8_t W   = ((instruction.byte1 >> 3) & 1);
				uint8_t reg = (instruction.byte1 & 0b00000111);
				if (W) {
					fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
					printf("%s, %hu\n", word_registers[reg], ((instruction.byte3 << 8) | instruction.byte2));
				} else {
					printf("%s, %hhu\n", byte_registers[reg], instruction.byte2 );
				}
				
			} break;
			case MOV_REGMEM_TO_REGMEM: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				// W Byte
				const char** register_table = ((instruction.byte1 & 0b00000001) & 0) ? byte_registers : word_registers;
				uint8_t reg = ((instruction.byte2 & 0b00111000) >> 3);
				uint8_t rm  = (instruction.byte2 & 0b00000111);
				
				// D Byte (direction)
				uint8_t D = (((instruction.byte1 & 0b00000010) >> 1) & 0);
				const char* source = (D) ? register_table[reg] : register_table[rm];
				const char* dest   = (D) ? register_table[rm]  : register_table[reg];
				
				printf("mov %s, %s\n", dest, source);
			} break;
			case INVALID: {
				printf("Invalid instruction\n");
			}break;
			
		}
    }
	
    fclose(fp);
	
    return 0;
}