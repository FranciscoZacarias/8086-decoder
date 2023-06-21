
void simulate_mov(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 = source.sImmediate;
			} else {
				destinationSimRegister->data8[destination.register_access.offset] = source.sImmediate;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];
			destinationSimRegister->data16 = sourceSimRegister->data16;
		} break;
	}

	fprintf(file, " %s: 0x%04hx (%d)", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_add(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 += source.uImmediate;

			} else {
				destinationSimRegister->data8[destination.register_access.offset] += source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			// TODO(fz): For add, sub and cmp, we need to check if this is a 8bit or 16bit.
			// Because, right now, this is just overriding the whole 16bit register, independent of the destination register size
			destinationSimRegister->data16 += sourceSimRegister->data16;
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_sub(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 -= source.uImmediate;
			} else {
				destinationSimRegister->data8[destination.register_access.offset] -= source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];
			// TODO(fz): For add, sub and cmp, we need to check if this is a 8bit or 16bit.
			// Because, right now, this is just overriding the whole 16bit register, independent of the destination register size
			destinationSimRegister->data16 -= sourceSimRegister->data16;
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_cmp(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];


	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.offset == 0 && destination.register_access.count == 2) {
				destinationSimRegister->data16 -= source.uImmediate;
			} else {
				destinationSimRegister->data8[destination.register_access.offset] -= source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];
			// TODO(fz): For add, sub and cmp, we need to check if this is a 8bit or 16bit.
			// Because, right now, this is just overriding the whole 16bit register, independent of the destination register size
			destinationSimRegister->data16 -= sourceSimRegister->data16;
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_instruction(SimRegister simRegisters[Register_count], Instruction instruction, FILE* file) {
	InstructionOperand destination = instruction.operands[0];
	InstructionOperand source      = instruction.operands[1];

	fprintf(file, " ; ");

	switch(instruction.operation) {
		case OP_mov: {
			simulate_mov(simRegisters, source, destination, file);
		} break;

		case OP_add: {
			simulate_add(simRegisters, source, destination, file);
		} break;

		case OP_sub: {
			simulate_sub(simRegisters, source, destination, file);
		} break;

		case OP_cmp: {
			simulate_cmp(simRegisters, source, destination, file);
		} break;
	}
}