InstructionFormat instruction_formats[] =
{
#include "instruction_table.inl"
};

InstructionOperand get_reg_operand(u32 intelRegIndex, b32 wide) {
	RegisterAccess regTable[][2] =
    {
        {{Register_A, 0, 1}, {Register_A,  0, 2}},
        {{Register_C, 0, 1}, {Register_C,  0, 2}},
        {{Register_D, 0, 1}, {Register_D,  0, 2}},
        {{Register_B, 0, 1}, {Register_B,  0, 2}},
        {{Register_A, 1, 1}, {Register_SP, 0, 2}},
        {{Register_C, 1, 1}, {Register_BP, 0, 2}},
        {{Register_D, 1, 1}, {Register_SI, 0, 2}},
        {{Register_B, 1, 1}, {Register_DI, 0, 2}},
    };
    
    InstructionOperand result = {};
    result.type            = OperandType_Register;
    result.register_access = regTable[intelRegIndex & 0x7][(wide != 0)];
    
    return result;
}

u32 parse_data_value(Memory* memory, SegmentedAccess* access, b32 exists, b32 wide, b32 signExtended) {
	u32 result = {};
	
	if (exists) {
		if (wide) {
			u8 d0 = read_memory(memory, get_absolute_address(*access, 0));
			u8 d1 = read_memory(memory, get_absolute_address(*access, 1));
			result = (d1 << 8) | d0;
			access->offset += 2;
		} else {
			result = read_memory(memory, get_absolute_address(*access));
			if (signExtended) {
				result = (s32)*(s8*)&result;
			}
			access->offset += 1;
		}
	}
	
	return result;
}

Instruction try_decode(Memory* memory, SegmentedAccess access, InstructionFormat* format) {
	
	Instruction instruction = {};
	u32 hasBits             = 0;
	u32 bits[Bits_Count]    = {};
	b32 isValidInstruction  = true;
	
	u32 startAddress = get_absolute_address(access);
	
	u8 bitsPendingCount = 0;
	u8 bitsPending      = 0;
	
	// We try to match the format to the segmentedaccess location
	for (u32 i = 0; (isValidInstruction && i < ArrayCount(format->bits)); i++) {
		InstructionBits testByte = format->bits[i];
		
		// NOTE(fz): When we run out of bits, usage attribtue will
		// default to Bits_Literal (0)
		if ((testByte.usage == Bits_Literal) && (testByte.bitCount == 0)) {
			break;
		}
		
		u32 readBits = testByte.value;
		if (testByte.bitCount != 0) {
			if (bitsPendingCount == 0) {
				bitsPendingCount = 8;
				bitsPending      = read_memory(memory, get_absolute_address(access));
				++access.offset;
			}
			
			assert(testByte.bitCount <= bitsPendingCount);
			
			// Keep only the necessairy bits 
			bitsPendingCount -= testByte.bitCount;
			readBits          = bitsPending;
			readBits        >>= bitsPendingCount;
			readBits         &= ~(0xff << testByte.bitCount);
		}
		
		if (testByte.usage == Bits_Literal) {
			isValidInstruction = isValidInstruction && (readBits == testByte.value);
		} else {
			bits[testByte.usage] |= readBits;
			hasBits              |= (1 << testByte.usage);
		}
	}
	
	if (isValidInstruction) {
		u32 mod = bits[Bits_MOD];
		u32 rm  = bits[Bits_RM];
		u32 W   = bits[Bits_W];
		u32 S   = bits[Bits_S];
		u32 D   = bits[Bits_D];
		
		b32 hasDirectAddress   = (mod == 0b00 && rm == 0b110);
		b32 hasDisplacement    = (bits[Bits_HasDisp] || mod == 0b10 || mod == 0b01 || hasDirectAddress);
		b32 isDisplacementWide = (bits[Bits_DispAlwaysW] || (mod == 0b10) || hasDirectAddress);
        b32 isDataW            = (bits[Bits_WMakesDataW] && !S && W);
		
		bits[Bits_Disp] |= parse_data_value(memory, &access, hasDisplacement, isDisplacementWide, !isDisplacementWide);
		bits[Bits_Data] |= parse_data_value(memory, &access, bits[Bits_HasData], isDataW, S);
		
		instruction.operation = format->type;
		instruction.address   = startAddress;
		instruction.size      = get_absolute_address(access) - startAddress;
		
		if (W) {
			instruction.flags |= Inst_Wide;
		}
		
		s16 displacement = (s16)(bits[Bits_Disp]);
		
		InstructionOperand *regOperand = &instruction.operands[D ? 0 : 1];
		InstructionOperand *modOperand = &instruction.operands[D ? 1 : 0];
		
		if (hasBits & (1 << Bits_REG)) {
			*regOperand = get_reg_operand(bits[Bits_REG], W);
		}
		
		if (hasBits & (1 << Bits_MOD)) {
			if (mod == 0b11) {
				*modOperand = get_reg_operand(rm, W || (bits[Bits_RMRegAlwaysW]));
			} else {
				modOperand->type                 = OperandType_Memory;
				modOperand->address.displacement = displacement;
				
				if (mod == 0b00 && rm == 0b110) {
					modOperand->address.base = EffectiveAddressBase_DirectAddress;
				} else {
					modOperand->address.base = (EffectiveAddressBase)(1+rm);
				}
			}
		}
		
		InstructionOperand *lastOperand = &instruction.operands[0];
		if (lastOperand->type) {
			lastOperand = &instruction.operands[1];
		}
		
		if (bits[Bits_RelJMPDisp]){
			lastOperand->type       = OperandType_RelativeImmediate;
			lastOperand->sImmediate = displacement + instruction.size;
		}
		
		if (bits[Bits_HasData]) {
			lastOperand->type       = OperandType_Immediate;
			lastOperand->uImmediate = bits[Bits_Data];
		}
	}
	
	return instruction;
}

Instruction decode_instruction(Memory* memory, SegmentedAccess* access) {
	
	Instruction instruction = {};
	
	// We loop each instruction format, and try to match it to
	// the current segmentedaccess location we have in memory
	for(u32 i = 0; i < ArrayCount(instruction_formats); i++) {
		InstructionFormat format = instruction_formats[i];
		instruction = try_decode(memory, *access, &format);
		
		if (instruction.operation) {
			access->offset += instruction.size;
			break;
		}
	}
	
	return instruction;
}
