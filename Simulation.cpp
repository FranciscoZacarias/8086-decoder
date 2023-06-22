
void set_flags(SimRegister flag_register, s16 result, u8 offset) {
	printf("  [Flags Set: ");

	if (result == 0) {
		printf("ZF");
		// SET ZF
		// UNSET CF SF
	} else if (result > 0) {
		// UNSET ZF CF SF
	} else if (result < 0) {
		printf("CF SF");
		// CF SF
		// UNSET ZF
	}
	printf("]");
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
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);
			if (source.register_access.count == 1) {
				destinationSimRegister->data8[source.register_access.offset] += source.uImmediate;
			} else if (source.register_access.count == 2) {
				destinationSimRegister->data16 += source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);

			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			// NOTE(fz): It's either 1 or 2 read count, I.e., 8 or 16bit register respectively
			if (source.register_access.count == 1) {
				destinationSimRegister->data8[source.register_access.offset] += sourceSimRegister->data8[source.register_access.offset];
			} else if (source.register_access.count == 2) {
				destinationSimRegister->data16 += sourceSimRegister->data16;
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);

	// NOTE(fz): If offset is 1, it gets the highest order bit of the first byte, if is 2, it gets the highest order bit of the second byte (the full 16bit register)
}

void simulate_sub(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);

			if (source.register_access.count == 1) {
				destinationSimRegister->data8[source.register_access.offset] -= source.uImmediate;
			} else if (source.register_access.count == 2) {
				destinationSimRegister->data16 -= source.uImmediate;
			}
		} break;

		case OperandType_Register: {
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			// NOTE(fz): It's either 1 or 2 read count, I.e., 8 or 16bit register respectively
			if (source.register_access.count == 1) {
				destinationSimRegister->data8[source.register_access.offset] -= sourceSimRegister->data8[source.register_access.offset];
			} else if (source.register_access.count == 2) {
				destinationSimRegister->data16 -= sourceSimRegister->data16;
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);

	// NOTE(fz): If offset is 1, it gets the highest order bit of the first byte, if is 2, it gets the highest order bit of the second byte (the full 16bit register)
}

void simulate_cmp(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s32 result = 0;

	fprintf(file, " %s: ", get_reg_name(destination.register_access));
	switch (source.type) {
		case OperandType_Immediate: {
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);
			result = (source.register_access.count == 1) ?
				(destinationSimRegister->data8[source.register_access.offset] - source.uImmediate) : (destinationSimRegister->data16 - source.uImmediate);

		} break;

		case OperandType_Register: {
			printf("[Register Offset: {Count: %d, Offset: %d}])", source.register_access.count, source.register_access.offset);
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			// NOTE(fz): It's either 1 or 2 read count, I.e., 8 or 16bit register respectively
			result = (source.register_access.count == 1) ?
				(destinationSimRegister->data8[source.register_access.offset] - sourceSimRegister->data8[source.register_access.offset]) : (destinationSimRegister->data16 - sourceSimRegister->data16);

		} break;
	}
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