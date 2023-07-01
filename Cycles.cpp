function u32 cycles_estimation_table(OperandType source, OperandType destination, OperationType operation) {
	// NOTE(fz): Effective Address Calculation component (Table 2-20)
	u32 EA = 0;

	// NOTE(fz): Table 2-21
	CycleReference mov_cycle_table[] = {
		{{ OperandType_Register, OperandType_Immediate }, 4},
		{{ OperandType_Register, OperandType_Register  }, 2},
		{{ OperandType_Register, OperandType_Memory    }, 8 + EA},
		{{ OperandType_Memory,   OperandType_Register  }, 9 + EA},
	};

	// NOTE(fz): Table 2-21
	CycleReference add_cycle_table[] = {
		{{ OperandType_Register, OperandType_Register  }, 3},
		{{ OperandType_Memory,   OperandType_Register  }, 16 + EA},
		{{ OperandType_Register, OperandType_Immediate }, 4},
	};

	CycleReference* table = mov_cycle_table;
	u32 table_count       = ArrayCount(mov_cycle_table);

	if (operation == OP_add) {
		table       = add_cycle_table;
		table_count = ArrayCount(add_cycle_table);
	}

	u32 result = 0;

	for (u32 i = 0; i < table_count; i++) {
		if (source      == table[i].operands[0] &&
			destination == table[i].operands[1]) {
			result = table[i].cycle_count;
		}
	}

	// NOTE(fz): I don't think that any instruction has 0 cycle cost.
	// Therefore, if we get a zero cycle, we know this operation
	// Is not on the table.
	return result;
}


u32 estimate_cycles(Instruction instruction) {

	// NOTE(fz): I will assume all pointers are in even addresses
	// Which could not be the case a lot of times!

	u32 cycles = 0;

	OperandType source      = instruction.operands[0].type;
	OperandType destination = instruction.operands[1].type;;

	cycles = cycles_estimation_table(source, destination, instruction.operation);
	printf("[Cycle Estimation: %02u] ", cycles);

	return cycles;
}