
////////////
// NOTE(fz): This simulation will only simulate 16 bit registers

void set_flags(SimRegister* flag_register, s16 result) {
	printf(" [Flags Set: ", result);

    b32 signBit = (result & 0b1000000000000000) != 0;
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

static u16 get_simulated_register_data_from_register(SimRegister simRegisters[Register_count], Register reg) {
	u16 data = 0;

	for(u32 i = 1; i < Register_count-1; i++) {
		if (simRegisters[i].reg == reg) {
			data = simRegisters[i].data16;
			break;
		}
	}

	return data;
}

static u16 get_register_data_from_address_base(SimRegister simRegisters[Register_count], EffectiveAddressBase base) {
	u16 result = 0;

	SimRegister simReg1 = { 0 };
	SimRegister simReg2 = { 0 };

	switch(base) {
		case EffectiveAddressBase_SI: {
			result = get_simulated_register_data_from_register(simRegisters, Register_SI);
		} break;

		case EffectiveAddressBase_DI: {
			result = get_simulated_register_data_from_register(simRegisters, Register_DI);
		} break;

		case EffectiveAddressBase_BP: {
			result = get_simulated_register_data_from_register(simRegisters, Register_BP);
		} break;

		case EffectiveAddressBase_BX: {
			result = get_simulated_register_data_from_register(simRegisters, Register_B);
		} break;

		case EffectiveAddressBase_BX_SI: {
			u16 dataB  = get_simulated_register_data_from_register(simRegisters, Register_B);
			u16 dataSI = get_simulated_register_data_from_register(simRegisters, Register_SI);
			result = dataB + dataSI;
		} break;

		case EffectiveAddressBase_BX_DI: {
			u16 dataB  = get_simulated_register_data_from_register(simRegisters, Register_B);
			u16 dataDI = get_simulated_register_data_from_register(simRegisters, Register_DI);
			result = dataB + dataDI;
		} break;

		case EffectiveAddressBase_BP_SI: {
			u16 dataBP = get_simulated_register_data_from_register(simRegisters, Register_BP);
			u16 dataSI = get_simulated_register_data_from_register(simRegisters, Register_SI);
			result = dataBP + dataSI;
		} break;

		case EffectiveAddressBase_BP_DI: {
			u16 dataBP = get_simulated_register_data_from_register(simRegisters, Register_BP);
			u16 dataDI = get_simulated_register_data_from_register(simRegisters, Register_DI);
			result = dataBP + dataDI;
		} break;

	}

	return result;
}

void simulate_mov(Memory* memory, SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);

	if (destination.type == OperandType_Memory) {

		// NOTE(fz): Now it's assuming that it's always a 2byte register;
		EffectiveAdressExpression address = destination.address;
		u16 data = get_register_data_from_address_base(simRegisters, address.base);

		memory->bytes[data + address.displacement]     = (source.uImmediate) & 0xFF;
		memory->bytes[data + address.displacement + 1] = (source.uImmediate >> 8) & 0xFF;

	} else {
		switch (source.type) {
			case OperandType_Immediate: {
				destinationSimRegister->data16 = source.sImmediate;
			} break;

			case OperandType_Register: {
				SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];
				destinationSimRegister->data16 = sourceSimRegister->data16;
			} break;

			case OperandType_Memory: {
				destinationSimRegister->data8[0] = memory->bytes[source.address.displacement];
				destinationSimRegister->data8[1] = memory->bytes[source.address.displacement + 1];
			} break;
		}
	}

	fprintf(file, " %s: 0x%04hx (%d)", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);
}

void simulate_add(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s16 result = 0;

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);

	switch (source.type) {
		case OperandType_Immediate: {
			destinationSimRegister->data16 += source.uImmediate;
			result = destinationSimRegister->data16;
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			destinationSimRegister->data16 += sourceSimRegister->data16;
			result = destinationSimRegister->data16;
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);
	set_flags(&simRegisters[Register_Flags], result);
}

void simulate_sub(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s16 result = 0;

	fprintf(file, " %s: 0x%04hx (%d) -> ", get_reg_name(destination.register_access), destinationSimRegister->data16, destinationSimRegister->data16);

	switch (source.type) {
		case OperandType_Immediate: {
			destinationSimRegister->data16 -= source.uImmediate;
			result = destinationSimRegister->data16;
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];

			destinationSimRegister->data16 -= sourceSimRegister->data16;
			result = destinationSimRegister->data16;
		} break;
	}

	fprintf(file, "0x%04hx (%d)", destinationSimRegister->data16, destinationSimRegister->data16);

	set_flags(&simRegisters[Register_Flags], result);
}

void simulate_cmp(SimRegister simRegisters[Register_count], InstructionOperand source, InstructionOperand destination, FILE* file) {
	SimRegister* destinationSimRegister = &simRegisters[destination.register_access.reg];
	s16 result = 0;

	switch (source.type) {
		case OperandType_Immediate: {
			result = destinationSimRegister->data16 - source.uImmediate;
		} break;

		case OperandType_Register: {
			SimRegister* sourceSimRegister = &simRegisters[source.register_access.reg];
			result = destinationSimRegister->data16 - sourceSimRegister->data16;
		} break;
	}

	set_flags(&simRegisters[Register_Flags], result);
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