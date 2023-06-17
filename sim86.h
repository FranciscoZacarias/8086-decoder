#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t b32;

enum OperationType {
	OP_none,
	
#define INSTRUCTION(Mnemonic, ...) OP_##Mnemonic,
#define INSTRUCTION_ALT(...)
#include "instruction_table.inl"
	
	OP_count
};

enum InstructionFlags {
    Inst_Wide    = (1 << 0),
};

enum EffectiveAddressBase {
	EffectiveAddressBase_DirectAddress,
	EffectiveAddressBase_BX_SI,
	EffectiveAddressBase_BX_DI,
	EffectiveAddressBase_BP_SI,
	EffectiveAddressBase_BP_DI,
	EffectiveAddressBase_SI,
	EffectiveAddressBase_DI,
	EffectiveAddressBase_BP,
	EffectiveAddressBase_BX,
	
	EffectiveAddressBase_count,
};

enum OperandType {
	OperandType_None,
	OperandType_Register,
	OperandType_Memory,
	OperandType_Immediate,
	OperandType_RelativeImmediate,
};

enum Register {
	Register_None,
	
	// A, B, C, D are also 16 bit registers.
	// X suffix means the whole 16bit register
	// L suffix is lower 8 bits
	// H suffix is higher 8 bits
	Register_A,
	Register_B,
	Register_C,
	Register_D,
	
	Register_SP,
	Register_BP,
	Register_SI,
	Register_DI,
	
	Register_count,
};

struct EffectiveAddressCalculation {
	EffectiveAddressBase base;
	s32 displacement;
};

struct RegisterAccess {
	Register reg;
	u8 offset; // How many bytes into the register
	u8 count;  // How many bytes from the offset
};

enum RegisterData {
	RegisterData_LOW = 0,
	RegisterData_HIGH,
};

struct SimulatedRegister {
	Register reg;
	
	union {
		u8  data8[2];
		u16 data16;
	};
};

struct EffectiveAdressExpression {
	EffectiveAddressBase base;
	s32 displacement;
};

struct InstructionOperand {
	OperandType type;
	
	union {
		EffectiveAdressExpression address;
		RegisterAccess register_access;
		u32 uImmediate;
		s32 sImmediate;
	};
};

struct Instruction {
	u32 address;
	u32 size;
	
	OperationType operation;
	u32 flags;
	
	InstructionOperand operands[2];
};
