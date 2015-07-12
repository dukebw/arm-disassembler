#include "allheads.h"

#define ARM_INSTRUCTION_LENGTH 4
#define THUMB_INSTRUCTION_LENGTH 2

#define MOV_IMMEDIATE 0x3A
#define LOAD_IMMEDIATE 0x59
#define BRANCH_EXCHANGE 0x12

#define PROGRAM_COUNTER_REGISTER 15
#define ROM_SIZE 0x100000
#define GENERAL_PURPOSE_REG_COUNT 31

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

#define ISOLATE_BIT(Register, Bit) (((Register) >> (Bit)) & 1)
#define N_BIT(Register) ISOLATE_BIT((Register), 31)
#define Z_BIT(Register) ISOLATE_BIT((Register), 30)
#define C_BIT(Register) ISOLATE_BIT((Register), 29)
#define V_BIT(Register) ISOLATE_BIT((Register), 28)
#define Q_BIT(Register) ISOLATE_BIT((Register), 27)

// TODO(brendan): put into arm_state struct (not global)
global_variable uint8 Rom[ROM_SIZE];
global_variable uint32 GeneralPurposeRegs[GENERAL_PURPOSE_REG_COUNT];
global_variable uint32 Cpsr;
global_variable uint32 Spsr;
global_variable uint32 ProgramCounter;

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

    // NOTE(brendan): Reset asserted and de-asserted
    ProgramCounter = 0;
	// NOTE(brendan): Supervisor mode | ARM state | disable fast interrupts and
	// normal interrupts
    // disable imprecise aborts (v6 only).
    // TODO(brendan): CPSR[9] = CP15_reg1_EEbit endianness on exception entry
    Cpsr = 0x1D3;

    // TODO(brendan): Thumb instructions
    uint32 InstructionLength = ARM_INSTRUCTION_LENGTH;
    uint32 NextInstruction;
    // TODO(brendan): condition on when to stop executing?
    for (ProgramCounter = 2*ARM_INSTRUCTION_LENGTH;
         ;
         ProgramCounter += InstructionLength)
    {
		uint32 RomIndex = ProgramCounter - 2*InstructionLength;
		if (InstructionLength == ARM_INSTRUCTION_LENGTH)
		{
			NextInstruction = *(uint32 *)(Rom + RomIndex);

			uint32 ConditionCode = NextInstruction >> 28;
			bool32 ConditionCodePassed = false;
			switch (ConditionCode)
			{
				case COND_EQ:
				{
					ConditionCodePassed = Z_BIT(Cpsr);
					break;
				}
				case COND_NE:
				{
					ConditionCodePassed = ~Z_BIT(Cpsr);
					break;
				}
				case COND_CS_HS:
				{
					ConditionCodePassed = C_BIT(Cpsr);
					break;
				}
				case COND_CC_LO:
				{
					ConditionCodePassed = ~C_BIT(Cpsr);
					break;
				}
				case COND_MI:
				{
					ConditionCodePassed = N_BIT(Cpsr);
					break;
				}
				case COND_PL:
				{
					ConditionCodePassed = ~N_BIT(Cpsr);
					break;
				}
				case COND_VS:
				{
					ConditionCodePassed = V_BIT(Cpsr);
					break;
				}
				case COND_VC:
				{
					ConditionCodePassed = ~V_BIT(Cpsr);
					break;
				}
				case COND_HI:
				{
					ConditionCodePassed = C_BIT(Cpsr) & ~Z_BIT(Cpsr);
					break;
				}
				case COND_LS:
				{
					ConditionCodePassed = ~C_BIT(Cpsr) | Z_BIT(Cpsr);
					break;
				}
				case COND_GE:
				{
					ConditionCodePassed = N_BIT(Cpsr) == V_BIT(Cpsr);
					break;
				}
				case COND_LT:
				{
					ConditionCodePassed = N_BIT(Cpsr) != V_BIT(Cpsr);
					break;
				}
				case COND_GT:
				{
					ConditionCodePassed = ~Z_BIT(Cpsr) &&
										  (N_BIT(Cpsr) == V_BIT(Cpsr));
					break;
				}
				case COND_LE:
				{
					ConditionCodePassed = Z_BIT(Cpsr) ||
										  (N_BIT(Cpsr) != V_BIT(Cpsr));
					break;
				}
				case COND_AL:
				{
					ConditionCodePassed = true;
					break;
				}
				default:
				{
					// NOTE(brendan): instruction doesn't depend on COND
					break;
				}
			}

			uint32 InstructionEncoding = (NextInstruction >> 20) & 0xFF;
			switch (InstructionEncoding)
			{
				case MOV_IMMEDIATE:
				{
					uint32 RotateImmediate = 2*((NextInstruction >> 8) & 0xF);
					uint32 ByteImmediate = NextInstruction & 0xFF;
					uint32 ShifterOperand =
						(ByteImmediate >> RotateImmediate) |
						(ByteImmediate << (32 - RotateImmediate));
					uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
					printf("mov r%d,#0x%x\n", RegisterIndex, ShifterOperand);
					GeneralPurposeRegs[RegisterIndex] = ShifterOperand;
					break;
				}
				case LOAD_IMMEDIATE:
				{
					uint32 BaseRegister = (NextInstruction >> 16) & 0xF;
					uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
					uint32 Offset = NextInstruction & 0xFFF;
					printf("ldr r%d,[r%d + #0x%x]\n", RegisterIndex, BaseRegister,
						   Offset);
					GeneralPurposeRegs[RegisterIndex] =
						*(uint32 *)(Rom + (GeneralPurposeRegs[BaseRegister] +
										   Offset));
					break;
				}
				// TODO(brendan): add other branches, determined by bits [4:7]
				case BRANCH_EXCHANGE:
				{
					uint32 RegisterIndex = NextInstruction & 0xF;
					printf("bx r%d\n", RegisterIndex);
					ProgramCounter = GeneralPurposeRegs[RegisterIndex] &
						0xFFFFFFFE;
					if (GeneralPurposeRegs[RegisterIndex] & 1)
					{
						InstructionLength = THUMB_INSTRUCTION_LENGTH;
					}
					break;
				}
				default:
				{
					Stopif(true, return 1, "Bad Instruction");
					break;
				}
			}
		}
		else
		{
		}
    }

    fclose(InputBinary);
}
