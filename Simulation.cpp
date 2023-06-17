
void simulate_instruction(SimRegister simRegisters[Register_count], Instruction instruction, FILE* file) {
	InstructionOperand destination = instruction.operands[0];
	InstructionOperand source      = instruction.operands[1];
	
	SimRegister destinationSimRegister = simRegisters[destination.register_access.reg];
	
	fprintf(file, "; %s: 0x%hx -> ", get_reg_name(destination.register_access), destinationSimRegister.data16);
	switch (source.type) {
		
		case OperandType_Immediate: {
			destinationSimRegister.data16 = source.sImmediate;
			fprintf(file, "0x%04hx", destinationSimRegister.data16);
		} break;
		
		
		case OperandType_Register: {
			//fprintf(file, "%s", get_reg_name(operand.register_access));
		} break;
		
	}
}
