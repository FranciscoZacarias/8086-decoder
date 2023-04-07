#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define LOW_8BITS(val) (val & 0b0000000011111111)
#define HIGH_8BITS(val) (val & 0b1111111100000000)

enum Operation_Code {
	INVALID = 0b00000000,
	
	// MOVs
	MOV_REGMEM_TOFROM_REG = 0b100010,
	MOV_IM_TO_REG         = 0b1011,
	
};

struct Instruction {
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
};

const char* byte_registers[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}; // W = 0, 8  bit registers
const char* word_registers[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}; // W = 1, 16 bit registers

// NOTE(fz): Second map contains displacement.
const char* address_calc[8] = {"[bx + si", "[bx + di", "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx"};


Operation_Code get_operation_code(uint8_t byte) {
	// NOTE(fz): Op codes always start from the upper bits
	if (((byte & 0b11110000) >> 4) == MOV_IM_TO_REG)         return MOV_IM_TO_REG;
	if (((byte & 0b11111100) >> 2) == MOV_REGMEM_TOFROM_REG) return MOV_REGMEM_TOFROM_REG;
	
	return INVALID;
}

// NOTE(fz):
// D = 0, Instruction source      is specified in the REG field
// D = 1, Instruction destination is specified in the REG field

void write_mov_reg_to_reg(uint8_t reg_bits, uint8_t rm_bits, uint8_t D_bit, const char** register_table) {
	if (D_bit) {
		printf("mov %s, %s\n", register_table[reg_bits], register_table[rm_bits]);
	} else {
		printf("mov %s, %s\n", register_table[rm_bits], register_table[reg_bits]);
	}
}

void write_mov_rm_tofrom_reg_no_displacement(uint8_t reg_bits, uint8_t rm_bits, uint8_t D_bit, const char** register_table) {
	if (D_bit) {
		printf("mov %s, %s]\n", register_table[reg_bits], address_calc[rm_bits]);
	} else {
		printf("mov %s], %s\n", address_calc[rm_bits], register_table[reg_bits]);
	}
}

// NOTE(fz): Here I'm passing the W (wide) value instead of the register table directly, because I'd still need to know
// the size of the registers to put in the format specifier of printf...
void write_mov_rm_tofrom_reg_with_displacement(uint8_t reg_bits, uint8_t rm_bits, uint8_t D_bit, uint8_t W, uint16_t data) {
	// NOTE(fz): W only specifies if registers are 8 or 16 bits. It's independent from the type of data we receive.
	const char** register_table = (W) ? word_registers : byte_registers;
	
	if (HIGH_8BITS(data) > 0) {
		if (!data) {
			if (D_bit) {
				printf("mov %s, %s]\n", register_table[reg_bits], address_calc[rm_bits]);
			} else {
				printf("mov %s], %s\n", address_calc[rm_bits], register_table[reg_bits]);
			}
		} else if (D_bit) {
			printf("mov %s, %s + %hu]\n", register_table[reg_bits], address_calc[rm_bits], data);
		} else {
			printf("mov %s + %hu], %s\n", address_calc[rm_bits], data, register_table[reg_bits]);
		}
	} else {
		if (!data) {
			if (D_bit) {
				printf("mov %s, %s]\n", register_table[reg_bits], address_calc[rm_bits]);
			} else {
				printf("mov %s], %s\n", address_calc[rm_bits], register_table[reg_bits]);
			}
		} else if (D_bit) {
			printf("mov %s, %s + %hhu]\n", register_table[reg_bits], address_calc[rm_bits], LOW_8BITS(data));
		} else {
			printf("mov %s + %hhu], %s\n", address_calc[rm_bits], LOW_8BITS(data), register_table[reg_bits]);
		}
	}
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

	printf("bits 16\n\n");
	
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
				
				uint8_t W   = ((instruction.byte1 >> 3) & 1);
				uint8_t reg = (instruction.byte1 & 0b00000111);
				
				if (W) {
					fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
					printf("mov %s, %hu\n", word_registers[reg], ((instruction.byte3 << 8) | instruction.byte2));
				} else {
					printf("mov %s, %hhu\n", byte_registers[reg], instruction.byte2 );
				}
				
			} break;
			
			case MOV_REGMEM_TOFROM_REG: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				uint8_t D = ((instruction.byte1 & 0b00000010) >> 1) & 1;
				uint8_t W =  (instruction.byte1 & 0b00000001) & 1;
				
				uint8_t mod = (instruction.byte2 & 0b11000000) >> 6;
				uint8_t reg = (instruction.byte2 & 0b00111000) >> 3;
				uint8_t rm  = (instruction.byte2 & 0b00000111);
				
				switch(mod) {
					
					// Memory mode, no displacement
					case 0b00: {
						// TODO(fz): *Except when R/M = 110, then it's 16 bit displacement
						const char** register_table = (W) ? word_registers : byte_registers;
						write_mov_rm_tofrom_reg_no_displacement(reg, rm, D, register_table);
					} break;
					
					// Memory mode, 8 bit displacement
					case 0b01: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						write_mov_rm_tofrom_reg_with_displacement(reg, rm, D, W, instruction.byte3);
					} break;
					
					// Memory mode, 16 bit displacement
					case 0b10: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						fread(&instruction.byte4, sizeof(instruction.byte4),  1, fp);
						write_mov_rm_tofrom_reg_with_displacement(reg, rm, D, W, ((instruction.byte4 << 8) | instruction.byte3));
					} break;
					
					// Register Mode, no displacement
					case 0b11: {
						const char** register_table = (W) ? word_registers : byte_registers;
						write_mov_reg_to_reg(reg, rm, D, register_table);
					} break;
				}
			} break;
			
			case INVALID: {
				printf("Invalid instruction\n");
			}break;
		}
    }
	
    fclose(fp);
	
    return 0;
}