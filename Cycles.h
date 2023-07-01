struct CycleReference {
	OperandType operands[2];
	u32 cycle_count;
};

u32 estimate_cycles(Instruction instruction);
