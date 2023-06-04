#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// NOTE(fz): For 16 bit values, obviously.
#define LOW_8BITS(val) (val & 0b0000000011111111)
#define HIGH_8BITS(val) (val & 0b1111111100000000)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int8_t   s8;
typedef int16_t  s16;

enum Operation_Code {
	// MOVs
	MOV_REGMEM_TOFROM_REG = 0b100010,
	MOV_IM_TO_REGMEM      = 0b1100011,
	MOV_IM_TO_REG         = 0b1011,
		
	// Aritmetic (ADD, SUB, CMP)
	ADD_REGMEM_WITH_REG_TO_EITHER = 0b000000,
	ADD_IM_TO_ACC                 = 0b0000010,
	SUB_REGMEM_WITH_REG_TO_EITHER = 0b001010,
	SUB_IM_TO_ACC                 = 0b0010110,
	CMP_REGMEM_AND_REG            = 0b001110,
	CMP_IM_TO_ACC                 = 0b0011110,
	ART_IM_WITH_REGMEM            = 0b100000, // Aritmetic op encoded in REG field
	
	INVALID = 0b11111111,
};

struct Instruction {
	u8 byte1;
	u8 byte2;
	u8 byte3;
	u8 byte4;
	u8 byte5;
	u8 byte6;
};

const char* byte_registers[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}; // W = 0, 8  bit registers
const char* word_registers[8] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}; // W = 1, 16 bit registers

// NOTE(fz): Second map contains displacement.
const char* address_calc[8] = {"[bx + si", "[bx + di", "[bp + si", "[bp + di", "[si", "[di", "[bp", "[bx"};

void print_byte(u8 byte) {
	for(s8 i = 7; i >= 0; i--) {
		printf("%d", (byte >> i) & 1);
	}
}

Operation_Code get_operation_code(u8 byte) {
	// NOTE(fz): Op codes always start from the upper bits
	if (((byte & 0b11111100) >> 2) == MOV_REGMEM_TOFROM_REG) return MOV_REGMEM_TOFROM_REG;
	if (((byte & 0b11111110) >> 1) == MOV_IM_TO_REGMEM)      return MOV_IM_TO_REGMEM;
	if (((byte & 0b11110000) >> 4) == MOV_IM_TO_REG)         return MOV_IM_TO_REG;
	
	if (((byte & 0b11111100) >> 2) == ADD_REGMEM_WITH_REG_TO_EITHER) return ADD_REGMEM_WITH_REG_TO_EITHER;
	if (((byte & 0b11111110) >> 1) == ADD_IM_TO_ACC)                 return ADD_IM_TO_ACC;
	
	if (((byte & 0b11111100) >> 2) == ART_IM_WITH_REGMEM)    return ART_IM_WITH_REGMEM; 
	
	return INVALID;
}

// NOTE(fz):
// D = 0, Instruction source      is specified in the REG field
// D = 1, Instruction destination is specified in the REG field

void write_reg_to_reg(const char instruction[3], u8 reg_bits, u8 rm_bits, u8 D_bit, const char** register_table) {
	if (D_bit) {
		printf("%s %s, %s\n", instruction, register_table[reg_bits], register_table[rm_bits]);
	} else {
		printf("%s %s, %s\n", instruction, register_table[rm_bits], register_table[reg_bits]);
	}
}

void write_rm_tofrom_reg_no_displacement(const char instruction[3], u8 reg_bits, u8 rm_bits, u8 D_bit, const char** register_table) {
	if (D_bit) {
		printf("%s %s, %s]\n", instruction, register_table[reg_bits], address_calc[rm_bits]);
	} else {
		printf("%s %s], %s\n", instruction, address_calc[rm_bits], register_table[reg_bits]);
	}
}

// NOTE(fz): Immediate wide should be either 1(16bit) or 0(8bit)
void write_rm_tofrom_immediate(const char instruction[3], u8 rm_bits, u8 W_bit, u16 displacement, u16 immediate) {
	if(W_bit) {
		if (displacement) {
			printf("%s word %s + %hu], %hu\n", instruction, address_calc[rm_bits], displacement, immediate);
		} else {
			printf("%s word %s], %hu\n", instruction, address_calc[rm_bits], immediate);
		}
	} else {
		if (displacement) {
			printf("%s byte %s + %hhu], %hhu\n", instruction, address_calc[rm_bits], displacement, LOW_8BITS(immediate));
		} else {
			printf("%s byte %s], %hhu\n", instruction, address_calc[rm_bits], LOW_8BITS(immediate));
		}
	}
}


// NOTE(fz): Here I'm passing the W (wide) value instead of the register table directly, because I'd still need to know
// the size of the registers to put in the format specifier of printf...
void write_rm_tofrom_reg_with_displacement(const char instruction[3], u8 reg_bits, u8 rm_bits, u8 D_bit, u8 W, u16 displacement) {
	// NOTE(fz): W only specifies if registers are 8 or 16 bits. It's independent from the type of displacement we receive.
	const char** register_table = (W) ? word_registers : byte_registers;
	if (HIGH_8BITS(displacement) > 0) {
		if (!displacement) {
			if (D_bit) {
				printf("%s %s, %s]\n", instruction,  register_table[reg_bits], address_calc[rm_bits]);
			} else {
				printf("%s %s], %s\n", instruction, address_calc[rm_bits], register_table[reg_bits]);
			}
		} else if (D_bit) {
			printf("%s %s, %s + %hu]\n", instruction, register_table[reg_bits], address_calc[rm_bits], displacement);
		} else {
			printf("%s %s + %hu], %s\n", instruction, address_calc[rm_bits], displacement, register_table[reg_bits]);
		}
	} else {
		if (!displacement) {
			if (D_bit) {
				printf("%s %s, %s]\n", instruction, register_table[reg_bits], address_calc[rm_bits]);
			} else {
				printf("%s %s], %s\n", instruction, address_calc[rm_bits], register_table[reg_bits]);
			}
		} else if (D_bit) {
			printf("%s %s, %s + %hhu]\n", instruction, register_table[reg_bits], address_calc[rm_bits], LOW_8BITS(displacement));
		} else {
			printf("%s %s + %hhu], %s\n", instruction, address_calc[rm_bits], LOW_8BITS(displacement), register_table[reg_bits]);
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
	
    struct Instruction instruction = { 0, 0 ,0 ,0, 0, 0 };
    while (fread(&instruction.byte1, sizeof(instruction.byte1), 1, fp) == 1) {
		// TODO(fz): Clean this up. Make sure we keep the first byte, but we should clean the other ones.
		instruction.byte2 = 0;
		instruction.byte3 = 0;	
		instruction.byte4 = 0;
		instruction.byte5 = 0;
		instruction.byte6 = 0;
		
		Operation_Code opcode = get_operation_code(instruction.byte1);
		
		switch(opcode) {
			
			case MOV_REGMEM_TOFROM_REG: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				u8 D = ((instruction.byte1 & 0b00000010) >> 1);
				u8 W =  (instruction.byte1 & 0b00000001);
				
				u8 mod = (instruction.byte2 & 0b11000000) >> 6;
				u8 reg = (instruction.byte2 & 0b00111000) >> 3;
				u8 rm  = (instruction.byte2 & 0b00000111);
				
				const char** register_table = (W) ? word_registers : byte_registers;
				switch(mod) {
					
					case 0b00: {
						if (rm == 0b110) {
							fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
							fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
							printf("mov %s, [%hu]\n", word_registers[reg], (instruction.byte4 << 8) | instruction.byte3);
						} else {
							write_rm_tofrom_reg_no_displacement("mov", reg, rm, D, register_table);
						}
					} break;
					
					// 8 bit displacement
					case 0b01: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						write_rm_tofrom_reg_with_displacement("mov", reg, rm, D, W, instruction.byte3);
					} break;
					
					// 16 bit displacement
					case 0b10: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						fread(&instruction.byte4, sizeof(instruction.byte4),  1, fp);
						u16 displacement = ((instruction.byte4 << 8) | instruction.byte3);
						write_rm_tofrom_reg_with_displacement("mov", reg, rm, D, W, displacement);
					} break;
					
					case 0b11: {
						write_reg_to_reg("mov", reg, rm, D, register_table);
					} break;
				}
			} break;
			
			case MOV_IM_TO_REGMEM: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				u8 W   = (instruction.byte1 & 0b00000001);
				
				// In this case, REG is always 0b000
				u8 mod = (instruction.byte2 & 0b11000000) >> 6;
				u8 rm  = (instruction.byte2 & 0b00000111);
				
				if (W) {
					// 16 bit displacement
					if (mod == 0b10) {
						fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
						fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
						fread(&instruction.byte5, sizeof(instruction.byte5), 1, fp);
						fread(&instruction.byte6, sizeof(instruction.byte6), 1, fp);
						
						u16 displacement = (instruction.byte4 << 8) | instruction.byte3;
						u16 data         = (instruction.byte6 << 8) | instruction.byte5;
						
						// TODO: word and byte can also be on the destination
						printf("mov [%s + %hu], word %hu\n", word_registers[rm], displacement, data);
					}
				} else {
					fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
					printf("mov %s], byte %hhu\n", address_calc[rm], instruction.byte3);
				}
			} break;
			
			case MOV_IM_TO_REG: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				u8 W   = ((instruction.byte1 >> 3) & 1);
				u8 reg = (instruction.byte1 & 0b00000111);
				
				if (W) {
					fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
					printf("mov %s, %hu\n", word_registers[reg], ((instruction.byte3 << 8) | instruction.byte2));
				} else {
					printf("mov %s, %hhu\n", byte_registers[reg], instruction.byte2 );
				}
				
			} break;
			
			case ADD_REGMEM_WITH_REG_TO_EITHER: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				u8 D = ((instruction.byte1 & 0b00000010) >> 1);
				u8 W =  (instruction.byte1 & 0b00000001);
				
				u8 mod = (instruction.byte2 & 0b11000000) >> 6;
				u8 reg = (instruction.byte2 & 0b00111000) >> 3;
				u8 rm  = (instruction.byte2 & 0b00000111);
				
				const char** register_table = (W) ? word_registers : byte_registers;
				switch(mod) {
					// write_rm_tofrom_reg_no_displacement(const char instruction[3], u8 reg_bits, u8 rm_bits, u8 D_bit, const char** register_table)
					
					case 0b00: {
						if (rm == 0b110) {
							fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
							fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
							printf("add %s, [%hu]\n", word_registers[reg], (instruction.byte4 << 8) | instruction.byte3);
						} else {
							write_rm_tofrom_reg_no_displacement("add", reg, rm, D, register_table);
						}
					} break;
					
					// 8 bit displacement
					case 0b01: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						write_rm_tofrom_reg_with_displacement("add", reg, rm, D, W, instruction.byte3);
					} break;
					
					// 16 bit displacement
					case 0b10: {
						fread(&instruction.byte3, sizeof(instruction.byte3),  1, fp);
						fread(&instruction.byte4, sizeof(instruction.byte4),  1, fp);
						u16 displacement = ((instruction.byte4 << 8) | instruction.byte3);
						write_rm_tofrom_reg_with_displacement("add", reg, rm, D, W, displacement);
					} break;
					
					case 0b11: {
						write_reg_to_reg("add", reg, rm, D, register_table);
					} break;
				}
			} break;
			
			case ADD_IM_TO_ACC: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				u8 W = (instruction.byte1 & 0b00000001);
				
				if (W) {
					fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
					u16 data = (instruction.byte3 << 8) | instruction.byte2;
					printf("add ax, %hu\n", data);
				} else {
					printf("add al, %hhu\n", instruction.byte2);
				}
			} break;
			
			case ART_IM_WITH_REGMEM: {
				fread(&instruction.byte2, sizeof(instruction.byte2), 1, fp);
				
				u8 S = ((instruction.byte1 & 0b00000010) >> 1);
				u8 W =  (instruction.byte1 & 0b00000001);
				
				u8 mod = (instruction.byte2 & 0b11000000) >> 6;
				u8 reg = (instruction.byte2 & 0b00111000) >> 3;
				u8 rm  = (instruction.byte2 & 0b00000111);
				
				// TODO(fz): SWitch through REG values to get operation, now it's just hardcoded ADD
				
				const char** register_table = (W) ? word_registers : byte_registers;
				switch(mod) {
					case 0b00: {
						if (rm == 0b110) {
							// 16bit displacement
							fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
							fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
							
							fread(&instruction.byte5, sizeof(instruction.byte5), 1, fp);
							if (W)  fread(&instruction.byte6, sizeof(instruction.byte6), 1, fp);
							
							u16 displacement = instruction.byte4 << 8 | instruction.byte3;
							u16 data         = instruction.byte6 << 8 | instruction.byte5;
							
							write_rm_tofrom_immediate("add", rm, W, displacement, data);
						} else {
							// No displacement							
							fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
							if (W)  fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
							
							u16 data = instruction.byte4 << 8 | instruction.byte3;
							
							write_rm_tofrom_immediate("add", rm, W, 0, data);
						}
					} break;
					
					// 8 bit displacement
					case 0b01: {
						fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
						
						fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
						if (W)  fread(&instruction.byte5, sizeof(instruction.byte5), 1, fp);
						
						u16 data = instruction.byte5 << 8 | instruction.byte4;
						
						write_rm_tofrom_immediate("add", rm, W, instruction.byte3, data);
					} break;
					
					// 16 bit displacement
					case 0b10: {
						// 16bit displacement
						fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
						fread(&instruction.byte4, sizeof(instruction.byte4), 1, fp);
						
						fread(&instruction.byte5, sizeof(instruction.byte5), 1, fp);
						if (W == 1 && S == 0)  fread(&instruction.byte6, sizeof(instruction.byte6), 1, fp);
						
						u16 displacement = instruction.byte4 << 8 | instruction.byte3;
						u16 data         = instruction.byte6 << 8 | instruction.byte5;
						
						write_rm_tofrom_immediate("add", rm, W, displacement, data);
					} break;
					
					// No displacement
					case 0b11: {
						fread(&instruction.byte3, sizeof(instruction.byte3), 1, fp);
						printf("add %s, %hhu\n", register_table[rm], instruction.byte3);
					} break;
				}
			} break;
			
			case INVALID: {
				printf("\nInvalid instruction: ");
				print_byte(instruction.byte1);
				printf("\n");
				return 0;
			} break;
		}
    }
	
    fclose(fp);
	
    return 0;
}
