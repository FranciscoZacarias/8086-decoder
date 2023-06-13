

#ifndef INSTRUCTION
#define INSTRUCTION(Mnemonic, Encoding, ...) { OP_##Mnemonic, Encoding, __VA_ARGS__ },
#endif

#ifndef INSTRUCTION_ALT
#define INSTRUCTION_ALT INSTRUCTION
#endif

#define BITS(bits) { Bits_Literal, sizeof(#bits)-1, 0b##bits }
#define D_BIT      { Bits_D, 1 }
#define S_BIT      { Bits_S, 1 }
#define W_BIT      { Bits_W, 1 }

#define MOD { Bits_MOD, 2 }
#define REG { Bits_REG, 3 }
#define RM  { Bits_RM,  3 }

#define IMPLICIT_S(Value)   { Bits_S,   0, Value }
#define IMPLICIT_D(Value)   { Bits_D,   0, Value }
#define IMPLICIT_W(value)   { Bits_W,   0, value }
#define IMPLICIT_REG(value) { Bits_REG, 0, value }
#define IMPLICIT_MOD(Value) { Bits_MOD, 0, Value }
#define IMPLICIT_RM(Value)  { Bits_RM,  0, Value }

#define DISP      { Bits_HasDIsp,     0, 1 }
#define ADDR      { Bits_HasDisp,     0, 1 }, { Bits_DispAlwaysW, 0, 1 } 
#define DATA      { Bits_HasData,     0, 1 }
#define DATA_IF_W { Bits_WMakesDataW, 0, 1 }


INSTRUCTION(mov,     { BITS(100010),  D_BIT, W_BIT,   MOD,  REG, RM })
INSTRUCTION_ALT(mov, { BITS(1100011), W_BIT,          MOD,  BITS(000), RM,          DATA, DATA_IF_W, IMPLICIT_D(0) })
INSTRUCTION_ALT(mov, { BITS(1011),    W_BIT, REG,     DATA, DATA_IF_W, IMPLICIT_D(1) })
INSTRUCTION_ALT(mov, { BITS(1010000), W_BIT,          ADDR, IMPLICIT_REG(0), IMPLICIT_MOD(0), IMPLICIT_RM(0b110), IMPLICIT_D(1) })
INSTRUCTION_ALT(mov, { BITS(1010001), W_BIT,          ADDR, IMPLICIT_REG(0), IMPLICIT_MOD(0), IMPLICIT_RM(0b110), IMPLICIT_D(0) })


#undef INSTRUCTION
#undef INSTRUCTION_ALT

#undef BITS
#undef D_BIT
#undef S_BIT
#undef W_BIT

#undef MOD
#undef REG
#undef RM

#undef IMPLICIT_S
#undef IMPLICIT_D
#undef IMPLICIT_W
#undef IMPLICIT_REG
#undef IMPLICIT_MOD
#undef IMPLICIT_RM

#undef DISP
#undef ADDR
#undef DATA
#undef DATA_IF_W