
struct SimRegister {
	Register reg; // NOTE(fz): This might be redundant if we're only keeping track of these registers in an array, where the index for each register is the enum for each register. But anyway.

	union {
		u8  data8[2];
		u16 data16;
	};
};

void simulate_instruction(SimRegister simRegisters[Register_count], Instruction instruction, s32* jump, FILE* file);
