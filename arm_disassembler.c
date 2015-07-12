#include "allheads.h"

#define ARM_INSTRUCTION_LENGTH 4
#define THUMB_INSTRUCTION_LENGTH 2

#define MOV_IMMEDIATE 0x3A
#define MOV_IMMEDIATE_S 0x3B
#define LOAD_IMMEDIATE 0x59
#define BRANCH_EXCHANGE 0x12
#define BRANCH 0xA
#define BRANCH_LINK 0xB

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
global_variable uint32 Cpsr;
global_variable uint32 Spsr;

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

	// NOTE(brendan): Reset asserted and de-asserted
	GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] = 0;
	// NOTE(brendan): Supervisor mode | ARM state | disable fast interrupts and
	// normal interrupts
	// disable imprecise aborts (v6 only).
	// TODO(brendan): CPSR[9] = CP15_reg1_EEbit endianness on exception entry
	Cpsr = MODE_SUPERVISOR;
	Cpsr |= DISABLE_FAST_INTERRUPTS_BIT_INDEX;
	Cpsr |= DISABLE_NORMAL_INTERRUPTS_BIT_INDEX;

	// TODO(brendan): Thumb instructions
	uint32 InstructionLength = ARM_INSTRUCTION_LENGTH;
	uint32 NextInstruction;
	for (GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] = 2*ARM_INSTRUCTION_LENGTH;
		 GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] < (RomLength + 2*ARM_INSTRUCTION_LENGTH);
		 GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] += InstructionLength)
	{
		uint32 RomIndex = GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] - 2*InstructionLength;
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
					ConditionCodePassed = !Z_BIT(Cpsr);
					break;
				}
				case COND_CS_HS:
				{
					ConditionCodePassed = C_BIT(Cpsr);
					break;
				}
				case COND_CC_LO:
				{
					ConditionCodePassed = !C_BIT(Cpsr);
					break;
				}
				case COND_MI:
				{
					ConditionCodePassed = N_BIT(Cpsr);
					break;
				}
				case COND_PL:
				{
					ConditionCodePassed = !N_BIT(Cpsr);
					break;
				}
				case COND_VS:
				{
					ConditionCodePassed = V_BIT(Cpsr);
					break;
				}
				case COND_VC:
				{
					ConditionCodePassed = !V_BIT(Cpsr);
					break;
				}
				case COND_HI:
				{
					ConditionCodePassed = C_BIT(Cpsr) && !Z_BIT(Cpsr);
					break;
				}
				case COND_LS:
				{
					ConditionCodePassed = !C_BIT(Cpsr) || Z_BIT(Cpsr);
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
					ConditionCodePassed = !Z_BIT(Cpsr) && (N_BIT(Cpsr) == V_BIT(Cpsr));
					break;
				}
				case COND_LE:
				{
					ConditionCodePassed = Z_BIT(Cpsr) || (N_BIT(Cpsr) != V_BIT(Cpsr));
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
				case MOV_IMMEDIATE_S:
				{
					uint32 RotateImmediate = 2*((NextInstruction >> 8) & 0xF);
					uint32 ByteImmediate = NextInstruction & 0xFF;
					uint32 ShifterOperand = (ByteImmediate >> RotateImmediate) |
											(ByteImmediate << (32 - RotateImmediate));
					uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
					printf("mov");
					printf(ConditionCodeStrings[ConditionCode]);
					if (S_BIT(NextInstruction))
					{
						printf("s");
					}
					printf(" r%d,#0x%x\n", RegisterIndex, ShifterOperand);
					if (ConditionCodePassed)
					{
						GeneralPurposeRegs[RegisterIndex] = ShifterOperand;
						if (S_BIT(NextInstruction))
						{
							if (RegisterIndex == PROGRAM_COUNTER_REGISTER)
							{
								// NOTE(brendan): User mode and System mode do not have an SPSR
								uint32 CurrentMode = (Cpsr & 0x1F);
								if ((CurrentMode != MODE_USER) && (CurrentMode != MODE_SYSTEM))
								{
									Cpsr = Spsr;
								}
								else
								{
									Stopif(true, return 1, "Current mode doesn't have SPSR and S bit set");
								}
							}
							else
							{
								SET_BIT(Cpsr, ISOLATE_BIT(GeneralPurposeRegs[RegisterIndex], 31), N_BIT_INDEX);
								uint32 NewZBitValue = (GeneralPurposeRegs[RegisterIndex] == 0) ? 1 : 0;
								SET_BIT(Cpsr, NewZBitValue, Z_BIT_INDEX);
								uint32 ShifterCarryOut = ISOLATE_BIT(ShifterOperand, 31);
								SET_BIT(Cpsr, ShifterCarryOut, C_BIT_INDEX);
							}
						}
					}
					break;
				}
				case LOAD_IMMEDIATE:
				{
					uint32 BaseRegister = (NextInstruction >> 16) & 0xF;
					uint32 RegisterIndex = (NextInstruction >> 12) & 0xF;
					uint32 Offset = NextInstruction & 0xFFF;
					printf("ldr r%d,[r%d + #0x%x]\n", RegisterIndex, BaseRegister, Offset);
					GeneralPurposeRegs[RegisterIndex] = *(uint32 *)(Rom + (GeneralPurposeRegs[BaseRegister] +
																		   Offset));
					break;
				}
				// TODO(brendan): add other branches, determined by bits [4:7]
				case BRANCH_EXCHANGE:
				{
					uint32 RegisterIndex = NextInstruction & 0xF;
					printf("bx r%d\n", RegisterIndex);
					GeneralPurposeRegs[PROGRAM_COUNTER_REGISTER] = GeneralPurposeRegs[RegisterIndex] &
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
}
