
void simulate_instruction(SimRegister simRegisters[Register_count], Instruction instruction, FILE* file) {
	InstructionOperand destination = instruction.operands[0];
	InstructionOperand source      = instruction.operands[1];
	
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	
	fprintf(file, " ; %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		
		case OperandType_Immediate: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 = source.uImmediate;
			} else {
				destinationSimRegister->data8[destination.register_access.offset] = source.uImmediate;
			}
			
			fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
		} break;
		
		case OperandType_Register: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 = source.uImmediate;
			} else {
				destinationSimRegister->data8[destination.register_access.offset] = source.uImmediate;
			}
			
			destinationSimRegister->data16 = source.sImmediate;
			fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
		} break;
	}
}