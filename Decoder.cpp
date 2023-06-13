InstructionFormat instruction_formats[] =
{
#include "instruction_table.inl"
};

Instruction try_decode(Memory* memory, SegmentedAccess access, InstructionFormat format) {
	
	Instruction instruction = {};
	u32 hasBits             = 0;
	u32 bits[Bits_Count]    = {};
	b32 isValidInstruction  = true;
	
	u32 startAddress = get_absolute_address(access);
	
	u8 bitsPendingCount = 0;
	u8 bitsPending      = 0;
	
	// We try to match the format to the segmentedaccess location
	for (u32 i = 0; (isValidInstruction && i < ArrayCount(format.bits)); i++) {
		InstructionBits testByte = format.bits[i];
		
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
		
	}
	
	return instruction;
}

Instruction decode_instruction(Memory* memory, SegmentedAccess* access) {
	
	Instruction instruction = {};
	
	// We loop each instruction format, and try to match it to
	// the current segmentedaccess location we have in memory
	for(u32 i = 0; i < ArrayCount(instruction_formats); i++) {
		InstructionFormat format = instruction_formats[i];
		instruction = try_decode(memory, *access, format);
		
		if (instruction.operation) {
			access->offset += instruction.size;
			break;
		}
	}
	
	return instruction;
}
