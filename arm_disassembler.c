#include "allheads.h"
#include "arm_disassembler.h"

global_variable char *
ConditionCodeStrings[] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", ""
};

// TODO(brendan): put into arm_state struct (not global)
global_variable uint8 Rom[ROM_SIZE];

internal uint32
ComputeShifterOperand(uint32 Instruction)
{
    uint32 RotateImmediate = 2*((Instruction >> 8) & 0xF);
    uint32 ByteImmediate = Instruction & 0xFF;
    uint32 ShifterOperand = (ByteImmediate >> RotateImmediate) |
                            (ByteImmediate << (32 - RotateImmediate));
    return ShifterOperand;
}

internal char *
EncodingToMnemonic(uint32 InstructionEncoding)
{
    static char *MnemonicArray[] = {
        "adc", "adcs", "add", "adds", "and", "ands", "bic", "bics", "cmn"
    };
    switch (InstructionEncoding)
    {
        case ADC_IMMEDIATE:
        case ADC:
        {
            return MnemonicArray[0];
        }
        case ADC_IMMEDIATE_S:
        case ADC_S:
        {
            return MnemonicArray[1];
        }
        case ADD_IMMEDIATE:
        case ADD:
        {
            return MnemonicArray[2];
        }
        case ADD_IMMEDIATE_S:
        case ADD_S:
        {
            return MnemonicArray[3];
        }
        case AND:
        case AND_IMMEDIATE:
        {
            return MnemonicArray[4];
        }
        case AND_S:
        case AND_IMMEDIATE_S:
        {
            return MnemonicArray[5];
        }
        case BIC:
        case BIC_IMMEDIATE:
        {
            return MnemonicArray[6];
        }
        case BIC_S:
        case BIC_IMMEDIATE_S:
        {
            return MnemonicArray[7];
        }
        case COMPARE_NEG:
        case COMPARE_NEG_IMMEDIATE:
        {
            return MnemonicArray[8];
        }
        default:
        {
            Stopif(true, return 0, "Bad InstructionEncoding");
            break;
        }
    }
}

internal bool32
IsCompareInstruction(uint32 InstructionEncoding)
{
    bool32 Result;
    if ((InstructionEncoding == COMPARE_NEG) || (InstructionEncoding == COMPARE_NEG_IMMEDIATE))
    {
        Result = true;
    }
    else
    {
        Result = false;
    }
    return Result;
}

// TODO(brendan): simulator?
int main(int argc, char **argv)
{
	Stopif(argc < 2, return 1, "No input");

	FILE *InputBinary = fopen(argv[1], "rb");
	Stopif(InputBinary == 0, return 1, "File does not exist");
	uint32 RomLength;
	for (RomLength = 0;
		 fread(Rom + RomLength, 1, 1, InputBinary);
		 ++RomLength)
	{
	}
	fclose(InputBinary);

	// TODO(brendan): Thumb instructions
	uint32 InstructionLength = ARM_INSTRUCTION_LENGTH;
	uint32 NextInstruction;
	for (uint32 RomIndex = 2*ARM_INSTRUCTION_LENGTH;
		 RomIndex < (RomLength + 2*ARM_INSTRUCTION_LENGTH);
		 RomIndex += InstructionLength)
	{
        NextInstruction = *(uint32 *)(Rom + RomIndex - 2*InstructionLength);
		if (InstructionLength == ARM_INSTRUCTION_LENGTH)
		{
			uint32 ConditionCode = NextInstruction >> 28;
			uint32 InstructionEncoding = (NextInstruction >> 20) & 0xFF;
            if ((GET_BITS(NextInstruction, 20, 31) == 0xE12) && (GET_BITS(NextInstruction, 4, 7) == 0x7))
            {
                printf("bkpt 0x%x\n", (NextInstruction & 0xF) | (GET_BITS(NextInstruction, 8, 19) << 4));
            }
            // TODO(brendan): CDP
            else if ((GET_BITS(NextInstruction, 24, 27) == 0xE) && (ISOLATE_BIT(NextInstruction, 4) == 0))
            {
                uint32 OpCode1 = GET_BITS(NextInstruction, 20, 23);
                uint32 OpCode2 = GET_BITS(NextInstruction, 5, 7);
                uint32 FirstOperand = GET_BITS(NextInstruction, 16, 19);
                uint32 SecondOperand = NextInstruction & 0xF;
                uint32 DestinationRegister = GET_BITS(NextInstruction, 12, 15);;
                uint32 Coprocessor = GET_BITS(NextInstruction, 8, 11);
                if (ConditionCode == 0xF)
                {
                    printf("cdp2");
                }
                else
                {
                    printf("cdp%s", ConditionCodeStrings[ConditionCode]);
                }
                printf(" p%d, %d, c%d, c%d, c%d, %d\n", Coprocessor, OpCode1, DestinationRegister, FirstOperand,
                                                        SecondOperand, OpCode2);
            }
            else if ((InstructionEncoding >> 5) == 0x5)
            {
                uint32 SignExtendedImmediate = NextInstruction & 0xFFFFFF;
                if (ISOLATE_BIT(SignExtendedImmediate, 23))
                {
                    SignExtendedImmediate |= 0xFF000000;
                }
                if (ConditionCode == 0xF)
                {
                    // TODO(brendan): set bit[1] of signed immediate to H bit
                    printf("blx #0x%x\n",
                           RomIndex + ((SignExtendedImmediate << 2) + (ISOLATE_BIT(NextInstruction, 24) << 1)));
                }
                else
                {
                    if (ISOLATE_BIT(NextInstruction, 24))
                    {
                        printf("bl");
                    }
                    else
                    {
                        printf("b");
                    }
                    printf("%s #0x%x\n", ConditionCodeStrings[ConditionCode],
                                         RomIndex + (SignExtendedImmediate << 2));
                }
            }
            else
            {
                switch (InstructionEncoding)
                {
                    case ADC_IMMEDIATE:
                    case ADC_IMMEDIATE_S:
                    case ADC:
                    case ADC_S:
                    case ADD_IMMEDIATE:
                    case ADD_IMMEDIATE_S:
                    case ADD:
                    case ADD_S:
                    case AND:
                    case AND_S:
                    case AND_IMMEDIATE:
                    case AND_IMMEDIATE_S:
                    case BIC:
                    case BIC_S:
                    case BIC_IMMEDIATE:
                    case BIC_IMMEDIATE_S:
                    case COMPARE_NEG:
                    case COMPARE_NEG_IMMEDIATE:
                    {
                        // TODO(brendan): implement?
                        Stopif(~ISOLATE_BIT(NextInstruction, 25) &
                               ISOLATE_BIT(NextInstruction, 4) &
                               ISOLATE_BIT(NextInstruction, 7),
                               return 1,
                               "Load/Store instruction extension space");
                        uint32 DestinationRegister = (NextInstruction >> 12) & 0xF;
                        uint32 FirstOperandRegister = (NextInstruction >> 16) & 0xF;
                        if (ISOLATE_BIT(NextInstruction, 25))
                        {
                            uint32 ShifterOperand = ComputeShifterOperand(NextInstruction);
                            if (IsCompareInstruction(InstructionEncoding))
                            {
                                printf("%s%s r%d, #0x%x\n", EncodingToMnemonic(InstructionEncoding),
                                                            ConditionCodeStrings[ConditionCode],
                                                            FirstOperandRegister,
                                                            ShifterOperand);
                            }
                            else
                            {
                                printf("%s%s r%d, r%d, #0x%x\n", EncodingToMnemonic(InstructionEncoding),
                                                                 ConditionCodeStrings[ConditionCode],
                                                                 DestinationRegister,
                                                                 FirstOperandRegister,
                                                                 ShifterOperand);
                            }
                        }
                        else if (GET_BITS(NextInstruction, 4, 11) == 0)
                        {
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            if (IsCompareInstruction(InstructionEncoding))
                            {
                                printf("%s%s r%d, r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                          ConditionCodeStrings[ConditionCode],
                                                          FirstOperandRegister,
                                                          SecondOperandRegister);
                            }
                            else
                            {
                                printf("%s%s r%d, r%d, r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                               ConditionCodeStrings[ConditionCode],
                                                               DestinationRegister,
                                                               FirstOperandRegister,
                                                               SecondOperandRegister);
                            }
                        }
                        else if (GET_BITS(NextInstruction, 4, 11) == 6)
                        {
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            if (IsCompareInstruction(InstructionEncoding))
                            {
                                printf("%s%s r%d, r%d, RRX\n", EncodingToMnemonic(InstructionEncoding),
                                                               ConditionCodeStrings[ConditionCode],
                                                               FirstOperandRegister,
                                                               SecondOperandRegister);
                            }
                            else
                            {
                                printf("%s%s r%d, r%d, r%d, RRX\n", EncodingToMnemonic(InstructionEncoding),
                                                                    ConditionCodeStrings[ConditionCode],
                                                                    DestinationRegister,
                                                                    FirstOperandRegister,
                                                                    SecondOperandRegister);
                            }
                        }
                        else if ((SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_LSL_IMMEDIATE) ||
                                 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_LSR_IMMEDIATE) ||
                                 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_ASR_IMMEDIATE) ||
                                 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_ROR_IMMEDIATE))
                        {
                            char *ShiftTypeString;
                            uint32 ShiftType = SHIFT_TYPE_IMMEDIATE(NextInstruction);
                            if (ShiftType == SHIFT_TYPE_LSL_IMMEDIATE)
                            {
                                ShiftTypeString = "LSL";
                            }
                            else if (ShiftType == SHIFT_TYPE_LSR_IMMEDIATE)
                            {
                                ShiftTypeString = "LSR";
                            }
                            else if (ShiftType == SHIFT_TYPE_ROR_IMMEDIATE)
                            {
                                ShiftTypeString = "ROR";
                            }
                            else
                            {
                                ShiftTypeString = "ASR";
                            }
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            uint32 ShiftImmediate = GET_BITS(NextInstruction, 7, 11);
                            if (IsCompareInstruction(InstructionEncoding))
                            {
                                printf("%s%s r%d, r%d %s #%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                 ConditionCodeStrings[ConditionCode],
                                                                 FirstOperandRegister,
                                                                 SecondOperandRegister,
                                                                 ShiftTypeString,
                                                                 ShiftImmediate);
                            }
                            else
                            {
                                printf("%s%s r%d, r%d, r%d %s #%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                      ConditionCodeStrings[ConditionCode],
                                                                      DestinationRegister,
                                                                      FirstOperandRegister,
                                                                      SecondOperandRegister,
                                                                      ShiftTypeString,
                                                                      ShiftImmediate);
                            }
                        }
                        else if ((SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_LSL_REGISTER) ||
                                 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_ASR_REGISTER) ||
                                 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_LSR_REGISTER) ||
                                 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_ROR_REGISTER))
                        {
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            uint32 ShiftRegister = GET_BITS(NextInstruction, 8, 11);
                            uint32 ShiftType = SHIFT_TYPE_REGISTER(NextInstruction);
                            char *ShiftTypeString;
                            if (ShiftType == SHIFT_TYPE_LSL_REGISTER)
                            {
                                ShiftTypeString = "LSL";
                            }
                            else if (ShiftType == SHIFT_TYPE_LSR_REGISTER)
                            {
                                ShiftTypeString = "LSR";
                            }
                            else if (ShiftType == SHIFT_TYPE_ROR_REGISTER)
                            {
                                ShiftTypeString = "ROR";
                            }
                            else
                            {
                                ShiftTypeString = "ASR";
                            }
                            if (IsCompareInstruction(InstructionEncoding))
                            {
                                printf("%s%s r%d, r%d %s r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                 ConditionCodeStrings[ConditionCode],
                                                                 FirstOperandRegister,
                                                                 SecondOperandRegister,
                                                                 ShiftTypeString,
                                                                 ShiftRegister);
                            }
                            else
                            {
                                printf("%s%s r%d, r%d, r%d %s r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                      ConditionCodeStrings[ConditionCode],
                                                                      DestinationRegister,
                                                                      FirstOperandRegister,
                                                                      SecondOperandRegister,
                                                                      ShiftTypeString,
                                                                      ShiftRegister);
                            }
                        }
                        break;
                    }
                    // TODO(brendan): Index instruction mnemonic in string array?
                    case BRANCH_EXCHANGE:
                    {
                        uint32 RegisterIndex = NextInstruction & 0xF;
                        if (GET_BITS(NextInstruction, 4, 7) == 0x3)
                        {
                            printf("blx%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
                        }
                        else if (GET_BITS(NextInstruction, 4, 7) == 0x2)
                        {
                            printf("bxj%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
                        }
                        else
                        {
                            printf("bx%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
                        }
                        break;
                    }
                    case COUNT_LEADING_ZEROS:
                    {
                        if (GET_BITS(NextInstruction, 4, 7) == 1)
                        {
                            printf("clz%s r%d, r%d\n", ConditionCodeStrings[ConditionCode],
                                                       GET_BITS(NextInstruction, 12, 15),
                                                       NextInstruction & 0xF);
                        }
                        else
                        {
                            Stopif(true, return 1, "Unsupported instruction");
                        }
                        break;
                    }
                    case LOAD_IMMEDIATE:
                    {
                        uint32 BaseRegister = (NextInstruction >> 16) & 0xF;
                        uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
                        uint32 Offset = NextInstruction & 0xFFF;
                        printf("ldr%s r%d,[r%d + #0x%x]\n", ConditionCodeStrings[ConditionCode],
                                                            RegisterIndex, BaseRegister, Offset);
                        break;
                    }
                    // TODO(brendan): Condition codes etc.
                    case MOV_IMMEDIATE:
                    case MOV_IMMEDIATE_S:
                    {
                        uint32 ShifterOperand  = ComputeShifterOperand(NextInstruction);
                        uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
                        printf("mov");
                        printf(ConditionCodeStrings[ConditionCode]);
                        if (S_BIT(NextInstruction))
                        {
                            printf("s");
                        }
                        printf(" r%d,#0x%x\n", RegisterIndex, ShifterOperand);
                        break;
                    }
                    case MRS_SPSR:
                    {
                        uint32 RegisterIndex = GET_BITS(NextInstruction, 12, 15);
                        printf("mrs r%d,SPSR\n", RegisterIndex);
                        break;
                    }
                    case MRS_CPSR:
                    {
                        uint32 RegisterIndex = GET_BITS(NextInstruction, 12, 15);
                        printf("mrs r%d,CPSR\n", RegisterIndex);
                        break;
                    }
                    default:
                    {
                        Stopif(true, return 1, "Bad Instruction");
                        break;
                    }
                }
            }
        }
        else
        {
        }
    }
}
