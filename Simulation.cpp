
void set_flags(SimRegister* flag_register, s8 result) {
	printf(" [Flags Set: ", result);

    b32 signBit = (result & 0b10000000) != 0;
    if (result == 0) {
        printf("ZF");
        flag_register->data16 |=   Flag_Zero;
        flag_register->data16 &= ~(Flag_Sign);
    } else if (signBit) {
        flag_register->data16 &= ~(Flag_Zero);
        flag_register->data16 &= ~(Flag_Sign);
    } else if (!signBit) {
        printf("SF");
        flag_register->data16 &= ~(Flag_Zero);
        flag_register->data16 |=   Flag_Sign;
    }
    printf("] ");
}

static SimRegister get_sim_register_from_address_base(SimRegister simRegisters[Register_count], EffectiveAddressBase base) {
	Register reg = Register_None;

	switch(base) {
		case EffectiveAddressBase_SI: {
			reg = Register_SI;
		} break;
		case EffectiveAddressBase_DI: {
			reg = Register_DI;
		} break;
		case EffectiveAddressBase_BP: {
			reg = Register_BP;
		} break;
		case EffectiveAddressBase_BX: {
			reg = Register_B;
		} break;
	}

	SimRegister simReg = simRegisters[0]; // Register_None...

	for(u32 i = 1; i < Register_count-1; i++) {
		if (simRegisters[i].reg == reg) {
			simReg = simRegisters[i];
			break;
		}
	}

	return simReg;
}

void simulate_mov(Memory* memory, SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);

	if (destination.type == OperandType_Memory) {

		// NOTE(fz): Now it's assuming that it's always a 2byte register;
		EffectiveAdressExpression address = destination.address;
		SimRegister simRegister = get_sim_register_from_address_base(simRegisters, address.base);

		memory->bytes[simRegister.data16 + address.displacement]     = (source.uImmediate) & 0xFF;
		memory->bytes[simRegister.data16 + address.displacement + 1] = (source.uImmediate >> 8) & 0xFF;

	} else {
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

			case OperandType_Memory: {
				// NOTE(fz): Assuming it's a 2byte register
				printf("{ memory->bytes[%d] = %d }", source.address.displacement, memory->bytes[source.address.displacement]);
				destinationSimRegister->data8[0] = memory->bytes[source.address.displacement];
				destinationSimRegister->data8[1] = memory->bytes[source.address.displacement + 1];
			} break;
		}
	}
	fprintf(file, " %s: 0x%04hx (%d)", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_add(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s8 result = 0;

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] += source.uImmediate;
				result = destinationSimRegister->data8[destination.register_access.offset];
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 += source.uImmediate;
				result = destinationSimRegister->data16;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] += sourceSimRegister->data8[source.register_access.offset];
				result = destinationSimRegister->data8[destination.register_access.offset];
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 += sourceSimRegister->data16;
				result = destinationSimRegister->data16;
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
	set_flags(&simRegisters[Register_Flags], result);
}

void simulate_sub(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s8 result = 0;

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
	switch (source.type) {
		case OperandType_Immediate: {
			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] -= source.uImmediate;
				result = destinationSimRegister->data8[destination.register_access.offset];
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 -= source.uImmediate;
				result = destinationSimRegister->data16;
			}
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			if (destination.register_access.count == 1) {
				destinationSimRegister->data8[destination.register_access.offset] -= sourceSimRegister->data8[source.register_access.offset];
				result = destinationSimRegister->data8[destination.register_access.offset];
			} else if (destination.register_access.count == 2) {
				destinationSimRegister->data16 -= sourceSimRegister->data16;
				result = destinationSimRegister->data16;
			}
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
	set_flags(&simRegisters[Register_Flags], result);
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

void simulate_instruction(Memory* memory, SimRegister simRegisters[Register_count], Instruction instruction, s32* jump, FILE* file) {
	InstructionOperand destination = instruction.operands[0];
	InstructionOperand source      = instruction.operands[1];

	fprintf(file, " ; ");

	switch(instruction.operation) {
		case OP_mov: {
			simulate_mov(memory, simRegisters, source, destination, file);
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

		case OP_jne: {
			if (!(simRegisters[Register_Flags].data16 & Flag_Zero)) {
				*jump += destination.sImmediate;
			}
		} break;
	}

	printf(" [IP: %04hx (%d)]", simRegisters[Register_IP].data16, simRegisters[Register_IP].data16);
}