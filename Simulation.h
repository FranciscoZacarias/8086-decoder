
struct SimRegister {
	Register reg;

	union {
		u8  data8[2];
		u16 data16;
	};
};

void simulate_instruction(Memory* memory, SimRegister simRegisters[Register_count], Instruction instruction, s32* jump, FILE* file);
