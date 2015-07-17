#include "allheads.h"

#define ARM_INSTRUCTION_LENGTH 4
#define THUMB_INSTRUCTION_LENGTH 2

#define ADC_IMMEDIATE 0x2A
#define ADC_IMMEDIATE_S 0x2B
#define ADC 0xA
#define ADC_S 0xB
#define ADD_IMMEDIATE 0x28
#define ADD_IMMEDIATE_S 0x29
#define ADD 0x8
#define ADD_S 0x9
#define BRANCH_EXCHANGE 0x12
#define BRANCH 0xA
#define BRANCH_LINK 0xB
#define LOAD_IMMEDIATE 0x59
#define MOV_IMMEDIATE 0x3A
#define MOV_IMMEDIATE_S 0x3B
#define MRS_CPSR 0x10
#define MRS_SPSR 0x14

#define LINK_REGISTER 14
#define PROGRAM_COUNTER_REGISTER 15
#define ROM_SIZE 0x100000
#define GENERAL_PURPOSE_REG_COUNT 32

#define COND_EQ 0x0
#define COND_NE 0x1
#define COND_CS_HS 0x2
#define COND_CC_LO 0x3
#define COND_MI 0x4
#define COND_PL 0x5
#define COND_VS 0x6
#define COND_VC 0x7
#define COND_HI 0x8
#define COND_LS 0x9
#define COND_GE 0xA
#define COND_LT 0xB
#define COND_GT 0xC
#define COND_LE 0xD
#define COND_AL 0xE

#define MODE_USER 0x10
#define MODE_FIQ 0x11
#define MODE_IRQ 0x12
#define MODE_SUPERVISOR 0x13
#define MODE_ABORT 0x17
#define MODE_UNDEFINED 0x1B
#define MODE_SYSTEM 0x1F

#define N_BIT_INDEX 31
#define Z_BIT_INDEX 30
#define C_BIT_INDEX 29
#define V_BIT_INDEX 28
#define Q_BIT_INDEX 27
#define S_BIT_INDEX 20
#define ARM_STATE_BIT_INDEX 5
#define DISABLE_FAST_INTERRUPTS_BIT_INDEX 6
#define DISABLE_NORMAL_INTERRUPTS_BIT_INDEX 7

// NOTE(brendan): data processing operand instructions
#define SHIFT_TYPE(Instruction) GET_BITS((Instruction), 4, 6)
#define SHIFT_TYPE_LSL 0
#define SHIFT_TYPE_LSR 2

#define GET_BITS(Register, StartBit, EndBit) \
    (((uint32)(Register) << (31 - (EndBit))) >> ((StartBit) + 31 - (EndBit)))
#define ISOLATE_BIT(Register, Bit) (((Register) >> (Bit)) & 1)
#define N_BIT(Register) ISOLATE_BIT((Register), N_BIT_INDEX)
#define Z_BIT(Register) ISOLATE_BIT((Register), Z_BIT_INDEX)
#define C_BIT(Register) ISOLATE_BIT((Register), C_BIT_INDEX)
#define V_BIT(Register) ISOLATE_BIT((Register), V_BIT_INDEX)
#define Q_BIT(Register) ISOLATE_BIT((Register), Q_BIT_INDEX)
#define S_BIT(Instruction) ISOLATE_BIT((Instruction), S_BIT_INDEX)

#define SET_BIT(Register, Value, BitToSet) (Register) |= (((Value) & 1) << (BitToSet))

global_variable char *
ConditionCodeStrings[] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", ""
};

// TODO(brendan): put into arm_state struct (not global)
global_variable uint8 Rom[ROM_SIZE];
global_variable uint32 GeneralPurposeRegs[GENERAL_PURPOSE_REG_COUNT];

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
        "adc", "adcs", "add", "adds"
    };
    switch (InstructionEncoding)
    {
        case ADC_IMMEDIATE:
        case ADC:
        {
            return MnemonicArray[0];
            break;
        }
        case ADC_IMMEDIATE_S:
        case ADC_S:
        {
            return MnemonicArray[1];
            break;
        }
        case ADD_IMMEDIATE:
        case ADD:
        {
            return MnemonicArray[2];
            break;
        }
        case ADD_IMMEDIATE_S:
        case ADD_S:
        {
            return MnemonicArray[3];
            break;
        }
        default:
        {
            Stopif(true, return 0, "Bad InstructionEncoding");
            break;
        }
    }
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
            if ((InstructionEncoding >> 5) == 0x5)
            {
                uint32 SignExtendedImmediate = NextInstruction & 0xFFFFFF;
                if (ISOLATE_BIT(SignExtendedImmediate, 23))
                {
                    SignExtendedImmediate |= 0xFF000000;
                }
                printf("b");
                if (ISOLATE_BIT(NextInstruction, 24))
                {
                    printf("l");
                }
                printf(" #0x%x\n", RomIndex + (SignExtendedImmediate << 2));
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
                            printf("%s%s r%d, r%d, #0x%x\n", EncodingToMnemonic(InstructionEncoding),
                                                             ConditionCodeStrings[ConditionCode],
                                                             DestinationRegister,
                                                             FirstOperandRegister,
                                                             ShifterOperand);
                        }
                        else if (GET_BITS(NextInstruction, 4, 11) == 0)
                        {
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            printf("%s%s r%d, r%d, r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                           ConditionCodeStrings[ConditionCode],
                                                           DestinationRegister,
                                                           FirstOperandRegister,
                                                           SecondOperandRegister);
                        }
                        // TODO(brendan): debug NextInstruction shifting to 0 for LSR
                        else if ((SHIFT_TYPE(NextInstruction) == SHIFT_TYPE_LSL) ||
                                 (SHIFT_TYPE(NextInstruction) == SHIFT_TYPE_LSR))
                        {
                            char ShiftTypeString[] = SHIFT_TYPE(NextInstruction) == SHIFT_TYPE_LSR ?
                                                     "LSR" : "LSL";
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            uint32 ShiftImmediate = GET_BITS(NextInstruction, 7, 11);
                            printf("%s%s r%d, r%d, r%d %s #%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                   ConditionCodeStrings[ConditionCode],
                                                                   DestinationRegister,
                                                                   FirstOperandRegister,
                                                                   SecondOperandRegister,
                                                                   ShiftTypeString,
                                                                   ShiftImmediate);
                        }
                        else if (GET_BITS(NextInstruction, 4, 7) == 1)
                        {
                            uint32 SecondOperandRegister = NextInstruction & 0xF;
                            uint32 ShiftRegister = GET_BITS(NextInstruction, 8, 11);
                            printf("%s%s r%d, r%d, r%d LSL r%d\n", EncodingToMnemonic(InstructionEncoding),
                                                                   ConditionCodeStrings[ConditionCode],
                                                                   DestinationRegister,
                                                                   FirstOperandRegister,
                                                                   SecondOperandRegister,
                                                                   ShiftRegister);
                        }
                        break;
                    }
                    // TODO(brendan): Index instruction mnemonic in string array?
                    case BRANCH_EXCHANGE:
                    {
                        uint32 RegisterIndex = NextInstruction & 0xF;
                        printf("bx");
                        printf(ConditionCodeStrings[ConditionCode]);
                        printf(" r%d\n", RegisterIndex);
                        GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] = GeneralPurposeRegs[RegisterIndex] &
                                                                       0xFFFFFFFE;
                        break;
                    }
                    case LOAD_IMMEDIATE:
                    {
                        uint32 BaseRegister = (NextInstruction >> 16) & 0xF;
                        uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
                        uint32 Offset = NextInstruction & 0xFFF;
                        printf("ldr r%d,[r%d + #0x%x]\n", RegisterIndex, BaseRegister, Offset);
                        break;
                    }
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
