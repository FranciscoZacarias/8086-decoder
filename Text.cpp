#include "Text.h"

char const *opcodeMnemonics[] = {
    "",

#define INSTRUCTION(Mnemonic, ...) #Mnemonic,
#define INSTRUCTION_ALT(...)
#include "instruction_table.inl"
};

char const *get_mnemonic(OperationType op) {
    char const *mnemonic = opcodeMnemonics[op];
    return mnemonic;
}

char const *get_reg_name(RegisterAccess access) {
    char const *names[][3] = {
        {"", "", ""},
        {"al", "ah", "ax"},
        {"bl", "bh", "bx"},
        {"cl", "ch", "cx"},
        {"dl", "dh", "dx"},
        {"sp", "sp", "sp"},
        {"bp", "bp", "bp"},
        {"si", "si", "si"},
        {"di", "di", "di"},
        {"flags", "flags", "flags"},
    };
    static_assert(ArrayCount(names) == Register_count, "Text table mismatch for register_index");

    char const *name = names[access.reg][(access.count == 2) ? 2 : access.offset&1];
    return name;
}

char const *get_effective_address_expression(EffectiveAdressExpression address) {
    char const *RMBase[] = {
        "",
        "bx+si",
        "bx+di",
        "bp+si",
        "bp+di",
        "si",
        "di",
        "bp",
        "bx",
    };
    static_assert(ArrayCount(RMBase) == EffectiveAddressBase_count, "Text table mismatch for effective_base_address");

    char const *result = RMBase[address.base];
    return result;
}

void print_instruction(Instruction instruction, FILE* file) {
	u32 flags = instruction.flags;
    u32 W     = flags & Inst_Wide;

    fprintf(file, "%s ", get_mnemonic(instruction.operation));

	char const *separator = "";

    for(u32 operandIndex = 0; operandIndex < ArrayCount(instruction.operands); ++operandIndex) {
        InstructionOperand operand = instruction.operands[operandIndex];
        if(operand.type != OperandType_None) {
            fprintf(file, "%s", separator);
            separator = ", ";

            switch(operand.type) {
                case OperandType_None: {} break;

                case OperandType_Register: {
                    fprintf(file, "%s", get_reg_name(operand.register_access));
                } break;

                case OperandType_Memory: {
                    EffectiveAdressExpression address = operand.address;

                    if(instruction.operands[0].type != OperandType_Register) {
                        fprintf(file, "%s ", W ? "word" : "byte");
                    }

                    fprintf(file, "[%s", get_effective_address_expression(address));
                    if(address.displacement != 0) {
                        fprintf(file, "%+d", address.displacement);
                    }
                    fprintf(file, "]");
                } break;

                case OperandType_Immediate: {
                    fprintf(file, "%d", operand.sImmediate);
                } break;

                case OperandType_RelativeImmediate: {
                    fprintf(file, "$%+d", operand.sImmediate);
                } break;
            }
        }
    }
}

