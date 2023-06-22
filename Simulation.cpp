
void set_flags(SimRegister* flag_register, s8 result) {
	printf(" [ RES:(%hhu) Flags Set: ", result);

	if (result == 0) {
		printf("ZF");
		flag_register->data16 |= Flag_Zero;
		flag_register->data16 &= ~(Flag_Carry);
		flag_register->data16 &= ~(Flag_Sign);
	} else if (result > 0) {
		flag_register->data16 &= ~(Flag_Zero);
		flag_register->data16 &= ~(Flag_Carry);
		flag_register->data16 &= ~(Flag_Sign);
	} else if (result < 0) {
		flag_register->data16 &= ~(Flag_Zero);
		flag_register->data16 |= Flag_Carry;
		flag_register->data16 |= Flag_Sign;
		printf("CF SF");
	}
	printf("] ");
}

void simulate_mov(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
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
			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] += source.uImmediate;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[destination.register_access.offset]);
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 += source.uImmediate;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[1]);
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] += sourceSimRegister->data8[source.register_access.offset];
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[destination.register_access.offset]);
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 += sourceSimRegister->data16;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[1]);
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_sub(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] -= source.uImmediate;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[destination.register_access.offset]);
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 -= source.uImmediate;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[1]);
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] -= sourceSimRegister->data8[source.register_access.offset];
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[destination.register_access.offset]);
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 -= sourceSimRegister->data16;
				set_flags(&simRegisters[Register_Flags], destinationSimRegister->data8[1]);
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);

	// NOTE(fz): If offset is 1, it gets the highest order bit of the first byte, if is 2, it gets the highest order bit of the second byte (the full 16bit register)
}

void simulate_cmp(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s32 result = 0;

	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.count == 1) {
				result = destinationSimRegister->data8[destination.register_access.offset] - source.uImmediate;
			} else if (destination.register_access.count == 2) {
				result = destinationSimRegister->data16 - source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			if (destination.register_access.count == 1) {
				result = destinationSimRegister->data8[destination.register_access.offset] - sourceSimRegister->data8[source.register_access.offset];
			} else if (destination.register_access.count == 2) {
				result = destinationSimRegister->data16 - sourceSimRegister->data16;
			}
		} break;
	}

	s8 sign_flag = (result > 0) ? 1 : (result < 0) ? -1 : 0;
	set_flags(&simRegisters[Register_Flags], sign_flag);
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