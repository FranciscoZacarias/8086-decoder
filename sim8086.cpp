#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

struct mov {
	uint8_t byte1;
	uint8_t byte2;
};

char* byte_registers[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}; // 8  bit registers
char* word_registers[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}; // 16 bit registers

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
	
	mov instruction;
    while (fread(&instruction.byte1, sizeof(instruction.byte1), 1, fp) == 1 && 
		   fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp) == 1) {
        uint8_t mov_instruction = 0b100010;
		if (((instruction.byte1 & 0b11111100) >> 2) == mov_instruction) {
			char** register_table = ((instruction.byte1 & 0b00000001) == 0) ? byte_registers : word_registers;
			uint8_t reg = ((instruction.byte2 & 0b00111000) >> 3);
			uint8_t rm  = (instruction.byte2 & 0b00000111);
			
			// D Byte (direction)
			uint8_t D = (((instruction.byte1 & 0b00000010) >> 1) == 0);
			char* source = (D) ? register_table[reg] : register_table[rm];
			char* dest   = (D) ? register_table[rm]  : register_table[reg];

			printf("mov %s, %s\n", dest, source);
		}
    }
	
    fclose(fp);
	
    return 0;
}