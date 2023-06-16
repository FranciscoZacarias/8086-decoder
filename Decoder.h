enum InstructionBitsUsage : u8 {
    Bits_Literal,
    Bits_MOD,
    Bits_REG,
    Bits_RM,
    Bits_SR,
    Bits_Disp,
    Bits_Data,

    Bits_HasDisp,
    Bits_DispAlwaysW,
    Bits_HasData,
    Bits_WMakesDataW,
    Bits_RMRegAlwaysW,
    Bits_RelJMPDisp,
    Bits_D,
    Bits_S,
    Bits_W,
    Bits_V,
    Bits_Z,
    
    Bits_Count,
};

struct InstructionBits {
	InstructionBitsUsage usage;
	u8 bitCount;
	u8 value;
};

struct InstructionFormat {
	OperationType type; // OP_ enum
	InstructionBits bits[16];
};