#include "allheads.h"
#include "arm_disassembler.h"

global_variable char *
ConditionCodeStrings[] =
{
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
    static char *MnemonicArray[] =
	{
        "adc", "adcs", "add", "adds", "and", "ands", "bic", "bics", "cmn", "cmp", "eor", "eors",
		"mov", "movs", "mvn", "mvns", "orr", "orrs", "rsb", "rsbs", "rsc", "rscs", "sbc", "sbcs"
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
		case COMPARE:
		case COMPARE_IMMEDIATE:
		{
			return MnemonicArray[9];
		}
		case EOR:
		case EOR_IMMEDIATE:
		{
			return MnemonicArray[10];
		}
		case EOR_S:
		case EOR_IMMEDIATE_S:
		{
			return MnemonicArray[11];
		}
		case MOV:
		case MOV_IMMEDIATE:
		{
			return MnemonicArray[12];
		}
		case MOV_S:
		case MOV_IMMEDIATE_S:
		{
			return MnemonicArray[13];
		}
		case MVN:
		case MVN_IMMEDIATE:
		{
			return MnemonicArray[14];
		}
		case MVN_S:
		case MVN_IMMEDIATE_S:
		{
			return MnemonicArray[15];
		}
		case ORR:
		case ORR_IMMEDIATE:
		{
			return MnemonicArray[16];
		}
		case ORR_S:
		case ORR_IMMEDIATE_S:
		{
			return MnemonicArray[17];
		}
		case RSB:
		case RSB_IMMEDIATE:
		{
			return MnemonicArray[18];
		}
		case RSB_S:
		case RSB_IMMEDIATE_S:
		{
			return MnemonicArray[19];
		}
		case RSC:
		case RSC_IMMEDIATE:
		{
			return MnemonicArray[20];
		}
		case RSC_S:
		case RSC_IMMEDIATE_S:
		{
			return MnemonicArray[21];
		}
		case SBC:
		case SBC_IMMEDIATE:
		{
			return MnemonicArray[22];
		}
		case SBC_S:
		case SBC_IMMEDIATE_S:
		{
			return MnemonicArray[23];
		}
        default:
        {
            Stopif(true, return 0, "Bad InstructionEncoding");
            break;
        }
    }
}

internal char *
GetShiftTypeString(uint32 NextInstruction, bool32 Immediate)
{
	char *Result;
	uint32 ShiftType = Immediate ? SHIFT_TYPE_IMMEDIATE(NextInstruction) :
								   SHIFT_TYPE_REGISTER(NextInstruction);
	if (!Immediate)
	{
		--ShiftType;
	}
	if (ShiftType == SHIFT_TYPE_LSL_IMMEDIATE)
	{
		Result = "LSL";
	}
	else if (ShiftType == SHIFT_TYPE_LSR_IMMEDIATE)
	{
		Result = "LSR";
	}
	else if (ShiftType == SHIFT_TYPE_ROR_IMMEDIATE)
	{
		Result = "ROR";
	}
	else
	{
		Result = "ASR";
	}
	return Result;
}
internal char *
GetLoadStoreString(uint32 NextInstruction)
{
	char *Result;
	Result = ISOLATE_BIT(NextInstruction, 20) ? "ldr" : "str";
	return Result;
}

internal void
ComputeSecondOperandString(char *SecondOperandString, uint32 NextInstruction)
{
	Stopif(SecondOperandString == 0, return, "Null SecondOperandString");
	if (ISOLATE_BIT(NextInstruction, 25))
	{
		uint32 ShifterOperand = ComputeShifterOperand(NextInstruction);
		snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "#0x%x",
				 ShifterOperand);
	}
	else if (GET_BITS(NextInstruction, 4, 11) == 0)
	{
		uint32 SecondOperandRegister = NextInstruction & 0xF;
		snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "r%d",
				 SecondOperandRegister);
	}
	else if (GET_BITS(NextInstruction, 4, 11) == 6)
	{
		uint32 SecondOperandRegister = NextInstruction & 0xF;
		snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "r%d, RRX",
				 SecondOperandRegister);
	}
	// TODO(brendan): shorter way to express this?
	else if ((SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_LSL_IMMEDIATE) ||
			 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_LSR_IMMEDIATE) ||
			 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_ASR_IMMEDIATE) ||
			 (SHIFT_TYPE_IMMEDIATE(NextInstruction) == SHIFT_TYPE_ROR_IMMEDIATE))
	{
		char *ShiftTypeString = GetShiftTypeString(NextInstruction, true);
		uint32 SecondOperandRegister = NextInstruction & 0xF;
		uint32 ShiftImmediate = GET_BITS(NextInstruction, 7, 11);
		snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "r%d %s #%d",
				 SecondOperandRegister, ShiftTypeString, ShiftImmediate);
	}
	else if ((SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_LSL_REGISTER) ||
			 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_ASR_REGISTER) ||
			 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_LSR_REGISTER) ||
			 (SHIFT_TYPE_REGISTER(NextInstruction) == SHIFT_TYPE_ROR_REGISTER))
	{
		uint32 SecondOperandRegister = NextInstruction & 0xF;
		uint32 ShiftRegister = GET_BITS(NextInstruction, 8, 11);
		char *ShiftTypeString = GetShiftTypeString(NextInstruction, false);
		snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "r%d %s r%d",
				 SecondOperandRegister, ShiftTypeString, ShiftRegister);
	}
	else
	{
		Stopif(true, return, "Invalid instruction format");
	}
}

internal void
ComputeFirstSecond(uint32 *FirstOperandRegister, uint32 *SecondOperandRegister, uint32 NextInstruction)
{
	Stopif((FirstOperandRegister == 0) || (SecondOperandRegister == 0), return,
		   "Bad SecondOperandRegister or FirstOperandRegister Arguments");
	*FirstOperandRegister = GET_BITS(NextInstruction, 16, 19);
	*SecondOperandRegister = NextInstruction & 0xF;
}

internal void
ComputeFirstDestination(uint32 *FirstOperandRegister, uint32 *DestinationRegister, uint32 NextInstruction)
{
	Stopif((FirstOperandRegister == 0) || (DestinationRegister == 0), return,
		   "Bad FirstOperandRegister or DestinationRegister Arguments");
	*DestinationRegister = GET_BITS(NextInstruction, 12, 15);
	*FirstOperandRegister = GET_BITS(NextInstruction, 16, 19);
}

internal void
ComputeFirstSecondDestination(uint32 *FirstOperandRegister, uint32 *SecondOperandRegister,
							  uint32 *DestinationRegister, uint32 NextInstruction)
{
	ComputeFirstSecond(FirstOperandRegister, SecondOperandRegister, NextInstruction);
	Stopif(DestinationRegister == 0, return, "Null destination register");
	*DestinationRegister = GET_BITS(NextInstruction, 12, 15);
}

internal void
ComputeSecondDestination(uint32 *SecondOperandRegister, uint32 *DestinationRegister, uint32 NextInstruction)
{
	Stopif((SecondOperandRegister == 0) || (DestinationRegister == 0), return,
		   "Bad SecondOperandRegister or DestinationRegister Arguments");
	*DestinationRegister = GET_BITS(NextInstruction, 12, 15);
	*SecondOperandRegister = NextInstruction & 0xF;
}

internal void
ComputeAddressingModeTwoOffset(char *OffsetString, uint32 NextInstruction)
{
	Stopif(OffsetString == 0, return, "Null OffsetString");
	char *Negative = ISOLATE_BIT(NextInstruction, 23) ? "" : "-";
	if (!ISOLATE_BIT(NextInstruction, 25))
	{
		uint32 RawOffset = NextInstruction & 0xFFF;
		snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "#%s0x%x", Negative, RawOffset);
	}
	else
	{
		uint32 SecondOperandRegister = NextInstruction & 0xF;
		if (GET_BITS(NextInstruction, 4, 11) == 0)
		{
			snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "%sr%d", Negative,
					 SecondOperandRegister);
		}
		else
		{
			if (GET_BITS(NextInstruction, 4, 11) == 0x6)
			{
				snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "%sr%d, RRX", Negative,
						 SecondOperandRegister);
			}
			else
			{
				uint32 ShiftImmediate = GET_BITS(NextInstruction, 7, 11);
				char *ShiftTypeString = GetShiftTypeString(NextInstruction, true);
				snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "%sr%d, %s #%d", Negative,
						 SecondOperandRegister, ShiftTypeString, ShiftImmediate);
			}
		}
	}
}

internal char *
GetAddressingModeFour(uint32 NextInstruction)
{
	char *Result;
	uint32 AddressingModeBits = GET_BITS(NextInstruction, 23, 24);
	if (AddressingModeBits == 0x1)
	{
		Result = "ia";
	}
	else if (AddressingModeBits == 0x3)
	{
		Result = "ib";
	}
	else if (AddressingModeBits == 0x0)
	{
		Result = "da";
	}
	else
	{
		Result = "db";
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
	// TODO(brendan): re-organize into alphabetical order
	uint32 InstructionLength = ARM_INSTRUCTION_LENGTH;
	uint32 NextInstruction;
	for (uint32 RomIndex = 2*ARM_INSTRUCTION_LENGTH;
		 RomIndex < (RomLength + 2*ARM_INSTRUCTION_LENGTH);
		 RomIndex += InstructionLength)
	{
        NextInstruction = *(uint32 *)(Rom + RomIndex - 2*InstructionLength);
		if (InstructionLength == ARM_INSTRUCTION_LENGTH)
		{
			uint32 FirstOperandRegister;
			uint32 SecondOperandRegister;
			uint32 DestinationRegister;
			uint32 ConditionCode = NextInstruction >> 28;
			uint32 InstructionEncoding = (NextInstruction >> 20) & 0xFF;
			uint32 SecondOpcode = GET_BITS(NextInstruction, 4, 7);
			char *InstructionMnemonic;
			char *AddressingModeFour;
			if ((GET_BITS(NextInstruction, 20, 31) == 0xE12) && (SecondOpcode == 0x7))
			{
				printf("bkpt 0x%x\n", (NextInstruction & 0xF) | (GET_BITS(NextInstruction, 8, 19) << 4));
			}
			else if ((InstructionEncoding == BRANCH_EXCHANGE) &&
					 ((SecondOpcode == 0x1) || (SecondOpcode == 0x2) || (SecondOpcode == 0x3)))
			{
				uint32 RegisterIndex = NextInstruction & 0xF;
				if (SecondOpcode == 0x3)
				{
					printf("blx%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
				}
				else if (SecondOpcode == 0x2)
				{
					printf("bxj%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
				}
				else
				{
					printf("bx%s r%d\n", ConditionCodeStrings[ConditionCode], RegisterIndex);
				}
			}
			else if ((GET_BITS(NextInstruction, 20, 31) == 0xF10) &&
					 (ISOLATE_BIT(NextInstruction, 16) == 0) &&
					 (ISOLATE_BIT(NextInstruction, 5) == 0))
			{
				uint32 ABit = ISOLATE_BIT(NextInstruction, 8);
				uint32 IBit = ISOLATE_BIT(NextInstruction, 7);
				uint32 FBit = ISOLATE_BIT(NextInstruction, 6);
				uint32 Mmod = ISOLATE_BIT(NextInstruction, 17);
				uint32 Imod = GET_BITS(NextInstruction, 18, 19);
				Stopif(((Mmod == 0) && !ISOLATE_BIT(Imod, 1)) || ((Imod == 0x1) && (Mmod == 0x1)),
					   return 1,
					   "Invalid cps opcode");
				printf("cps");
				if (Imod & 0x2)
				{
					char *Effect = (Imod & 0x1) ? "id" : "ie";
					char *ABitString = ABit ? "a" : "";
					char *IBitString = IBit ? "i" : "";
					char *FBitString = FBit ? "f" : "";
					printf("%s %s%s%s%s", Effect, ABitString, IBitString, FBitString,
										  Mmod ? "," : "");
				}
				if (Mmod == 0x1)
				{
					printf(" #%d", NextInstruction & 0x1F);
				}
				printf("\n");
			}
			else if (((GET_BITS(NextInstruction, 25, 27) == 0x6) && ISOLATE_BIT(NextInstruction, 20)) &&
					 (InstructionEncoding != MRRC))
			{
				uint32 Coprocessor = GET_BITS(NextInstruction, 8, 11);
				uint32 CoprocessorRegister = GET_BITS(NextInstruction, 12, 15);
				uint32 Register = GET_BITS(NextInstruction, 16, 19);
				uint32 WBit = ISOLATE_BIT(NextInstruction, 21);
				uint32 NBit = ISOLATE_BIT(NextInstruction, 22);
				char *LString = NBit ? "l" : "";
				if (ConditionCode == 0xF)
				{
					printf("ldc2%s p%d, c%d, ", LString, Coprocessor, CoprocessorRegister);
				}
				else
				{
					printf("ldc%s%s p%d, c%d, ", ConditionCodeStrings[ConditionCode], LString, Coprocessor,
											     CoprocessorRegister);
				}
				char *AddressingMode;
				uint32 Offset = (NextInstruction & 0xFF) << 2;
				Offset = ISOLATE_BIT(NextInstruction, 23) ? Offset : -Offset;
				if (ISOLATE_BIT(NextInstruction, 24))
				{
					AddressingMode = WBit ? "[r%d, #%d]!\n" : "[r%d, #%d]\n";
				}
				else
				{
					AddressingMode = WBit ? "[r%d], #%d\n" : "[r%d], {#%d}\n";
				}
				printf(AddressingMode, Register, Offset);
			}
			else if ((GET_BITS(NextInstruction, 25, 27) == 0x4) && ISOLATE_BIT(NextInstruction, 20))
			{
				AddressingModeFour = GetAddressingModeFour(NextInstruction);
				bool32 WriteModifiedBack = ISOLATE_BIT(NextInstruction, 21);
				if ((GET_BITS(NextInstruction, 25, 31) == 0x7C) && (!ISOLATE_BIT(NextInstruction, 22)) &&
					ISOLATE_BIT(NextInstruction, 20) && (GET_BITS(NextInstruction, 8, 11) == 0xA))
				{
					printf("rfe%s r%d%s\n", AddressingModeFour, GET_BITS(NextInstruction, 16, 19),
						   WriteModifiedBack ? "!" : "");
				}
				else
				{
					uint32 BaseRegister = (NextInstruction >> 16) & 0xF;
					printf("ldm%s%s r%d%s, {", ConditionCodeStrings[ConditionCode], AddressingModeFour,
						   BaseRegister, WriteModifiedBack ? "!" : "");
					bool32 FirstRegister = true;
					for (uint32 RegisterIndex = 0;
						 RegisterIndex < GENERAL_PURPOSE_REG_COUNT;
						 ++RegisterIndex)
					{
						if (ISOLATE_BIT(NextInstruction, RegisterIndex))
						{
							if (FirstRegister)
							{
								FirstRegister = false;
								printf("r%d", RegisterIndex);
							}
							else
							{
								printf(", r%d", RegisterIndex);
							}
						}
					}
					printf("}%s\n", ISOLATE_BIT(NextInstruction, 22) ? "^" : "");
				}
			}
            else if ((GET_BITS(NextInstruction, 24, 27) == 0xE) && (ISOLATE_BIT(NextInstruction, 4) == 0))
            {
                uint32 OpCode1 = GET_BITS(NextInstruction, 20, 23);
                uint32 OpCode2 = GET_BITS(NextInstruction, 5, 7);
				ComputeFirstSecondDestination(&FirstOperandRegister, &SecondOperandRegister, &DestinationRegister,
											  NextInstruction);
                uint32 Coprocessor = GET_BITS(NextInstruction, 8, 11);
                if (ConditionCode == 0xF)
                {
                    printf("cdp2");
                }
                else
                {
                    printf("cdp%s", ConditionCodeStrings[ConditionCode]);
                }
                printf(" p%d, %d, c%d, c%d, c%d, %d\n", Coprocessor, OpCode1, DestinationRegister,
														FirstOperandRegister, SecondOperandRegister, OpCode2);
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
					printf("blx #0x%x\n", RomIndex + ((SignExtendedImmediate << 2) +
													  (ISOLATE_BIT(NextInstruction, 24) << 1)));
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
			else if (QDADD_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QADD_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QADD16_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QADD8_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QADDSUBX_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QDSUB_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QSUB_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QSUB16_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QSUB8_CONDITION(InstructionEncoding, SecondOpcode) ||
					 QSUBADDX_CONDITION(InstructionEncoding, SecondOpcode) ||
					 SADD16_CONDITION(InstructionEncoding, SecondOpcode) ||
					 SADD8_CONDITION(InstructionEncoding, SecondOpcode) ||
					 SADDSUBX_CONDITION(InstructionEncoding, SecondOpcode))
			{
				if (QDADD_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qdadd";
				}
				else if (QADD_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qadd";
				}
				else if (QADD16_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qadd16";
				}
				else if (QADD8_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qadd8";
				}
				else if (QADDSUBX_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qaddsubx";
				}
				else if (QDSUB_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qdsub";
				}
				else if (QSUB_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qsub";
				}
				else if (QSUB16_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qsub16";
				}
				else if (QSUB8_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qsub8";
				}
				else if (QSUBADDX_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "qsubaddx";
				}
				else if (SADD16_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "sadd16";
				}
				else if (SADD8_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "sadd8";
				}
				else if (SADDSUBX_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "saddsubx";
				}
				else
				{
					Stopif(true, return 1, "Bad qadd instruction");
				}
				ComputeFirstSecondDestination(&FirstOperandRegister, &SecondOperandRegister, &DestinationRegister,
											  NextInstruction);
				printf("%s%s r%d, r%d, r%d\n", InstructionMnemonic, ConditionCodeStrings[ConditionCode],
					   DestinationRegister, SecondOperandRegister, FirstOperandRegister);
			}
			// TODO(brendan): compress with addressing mode 2?
			else if ((GET_BITS(NextInstruction, 25, 27) == 0) && ISOLATE_BIT(NextInstruction, 7) &&
					 ISOLATE_BIT(NextInstruction, 4))
			{
				if ((InstructionEncoding == 0x19) && (SecondOpcode == 0x9))
				{
					printf("ldrex%s r%d, [r%d]\n", ConditionCodeStrings[ConditionCode],
						   (NextInstruction >> 12) & 0xF, (NextInstruction >> 16) & 0xF);
				}
				else if ((GET_BITS(NextInstruction, 21, 27) == 0x1) &&
						 (SecondOpcode == 0x9))
				{
					uint32 DestinationRegister = GET_BITS(NextInstruction, 16, 19);
					uint32 AddRegisterN = GET_BITS(NextInstruction, 12, 15);
					uint32 MultiplyRegisterM = NextInstruction & 0xF;
					uint32 MultiplyRegisterS = GET_BITS(NextInstruction, 8, 11);
					printf("mla%s%s r%d, r%d, r%d, r%d\n", ConditionCodeStrings[ConditionCode],
						   ISOLATE_BIT(NextInstruction, 20) ? "s" : "",
						   DestinationRegister, MultiplyRegisterM, MultiplyRegisterS, AddRegisterN);
				}
				else if ((GET_BITS(NextInstruction, 21, 27) == 0) && (SecondOpcode == 0x9))
				{
					char *UpdateStatusString = ISOLATE_BIT(NextInstruction, 20) ? "s" : "";
					printf("mul%s%s r%d, r%d, r%d\n", ConditionCodeStrings[ConditionCode], UpdateStatusString,
						   GET_BITS(NextInstruction, 16, 19), NextInstruction & 0xF,
						   GET_BITS(NextInstruction, 8, 11));
				}
				else
				{
					char FirstWord[MAX_OFFSET_STRING_LENGTH];
					uint32 LBit = ISOLATE_BIT(NextInstruction, 20);
					uint32 SBit = ISOLATE_BIT(NextInstruction, 6);
					uint32 HBit = ISOLATE_BIT(NextInstruction, 5);
					if (!LBit && !SBit && HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "str%sh",
								 ConditionCodeStrings[ConditionCode]);
					}
					else if (!LBit && SBit && !HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "ldr%sd",
								 ConditionCodeStrings[ConditionCode]);
					}
					else if (!LBit && SBit && HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "str%sd",
								 ConditionCodeStrings[ConditionCode]);
					}
					else if (LBit && !SBit && HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "ldr%sh",
								 ConditionCodeStrings[ConditionCode]);
					}
					else if (LBit && SBit && !HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "ldr%ssb",
								 ConditionCodeStrings[ConditionCode]);
					}
					else if (LBit && SBit && HBit)
					{
						snprintf(FirstWord, MAX_OFFSET_STRING_LENGTH, "ldr%ssh",
								 ConditionCodeStrings[ConditionCode]);
					}
					else
					{
						Stopif(true, return 1, "Bad instruction mnemonic (ldrsh type)");
					}
					ComputeFirstDestination(&FirstOperandRegister, &DestinationRegister, NextInstruction);
					uint32 PBit = ISOLATE_BIT(NextInstruction, 24);
					uint32 UBit = ISOLATE_BIT(NextInstruction, 23);
					uint32 ImmediateBit = ISOLATE_BIT(NextInstruction, 22);
					uint32 WBit = ISOLATE_BIT(NextInstruction, 21);
					char OffsetString[MAX_OFFSET_STRING_LENGTH];
					char *Negative = UBit ? "" : "-";
					char *PreIndexedString = "";
					if (WBit && PBit)
					{
						PreIndexedString = "!";
					}
					if (ImmediateBit)
					{
						uint32 Offset = (GET_BITS(NextInstruction, 8, 11) << 4) | (NextInstruction & 0xF);
						snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "#%s0x%x", Negative, Offset);
					}
					else if (!ImmediateBit)
					{
						uint32 SecondOperandRegister = NextInstruction & 0xF;
						snprintf(OffsetString, MAX_OFFSET_STRING_LENGTH, "%sr%d", Negative,
								 SecondOperandRegister);
					}
					else
					{
						Stopif(true, return 1, "Illegal addressing mode 3 instruction");
					}
					if (PBit)
					{
						printf("%s r%d, [r%d, %s]%s\n", FirstWord, DestinationRegister,
							   FirstOperandRegister, OffsetString, PreIndexedString);
					}
					else
					{
						printf("%s r%d, [r%d], %s\n", FirstWord, DestinationRegister,
							   FirstOperandRegister, OffsetString);
					}
				}
			}
			else if ((GET_BITS(NextInstruction, 24, 27) == 0xE) && ISOLATE_BIT(NextInstruction, 4))
			{
				uint32 OpCode1 = GET_BITS(NextInstruction, 21, 23);
				uint32 OpCode2 = GET_BITS(NextInstruction, 5, 7);
				uint32 Coprocessor = GET_BITS(NextInstruction, 8, 11);
				uint32 SourceRegister = GET_BITS(NextInstruction, 12, 15);
				uint32 DestinationCpReg = GET_BITS(NextInstruction, 16, 19);
				uint32 ExtraCpReg = NextInstruction & 0xF;
				InstructionMnemonic = ISOLATE_BIT(NextInstruction, 20) ? "mrc" : "mcr";
				if (ConditionCode == 0xF)
				{
					printf("%s2 p%d, %d, r%d, c%d, c%d, %d\n", InstructionMnemonic, Coprocessor, OpCode1,
						   SourceRegister, DestinationCpReg, ExtraCpReg, OpCode2);
				}
				else
				{
					printf("%s%s p%d, %d, r%d, c%d, c%d, %d\n", InstructionMnemonic,
						   ConditionCodeStrings[ConditionCode], Coprocessor, OpCode1, SourceRegister,
						   DestinationCpReg, ExtraCpReg, OpCode2);
				}
			}
			else if (((InstructionEncoding == MSR_IMMEDIATE_CPSR) ||
					  (InstructionEncoding == MSR_IMMEDIATE_SPSR)) ||
					 (((InstructionEncoding == MSR_REGISTER_CPSR) ||
					   (InstructionEncoding == MSR_REGISTER_SPSR)) && (SecondOpcode == 0)))
			{
				char *StatusRegister = ISOLATE_BIT(NextInstruction, 22) ? "spsr" : "cpsr";
				char FieldsString[] = "cxsf";
				char Fields[MSR_FIELDS_COUNT + 1] = {};
				uint32 FieldMask = GET_BITS(NextInstruction, 16, 19);
				for (uint32 FieldsStringIndex = 0, FieldsIndex = 0;
					 FieldsStringIndex < MSR_FIELDS_COUNT;
					 ++FieldsStringIndex)
				{
					if (FieldMask & (1 << FieldsStringIndex))
					{
						Fields[FieldsIndex++] = FieldsString[FieldsStringIndex];
					}
				}
				char SecondOperandString[MAX_OFFSET_STRING_LENGTH];
				if ((InstructionEncoding == MSR_IMMEDIATE_CPSR) || (InstructionEncoding == MSR_IMMEDIATE_SPSR))
				{
					snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "#0x%x",
							 ComputeShifterOperand(NextInstruction));
				}
				else
				{
					snprintf(SecondOperandString, MAX_OFFSET_STRING_LENGTH, "r%d", NextInstruction & 0xF);
				}
				printf("msr%s %s_%s, %s\n", ConditionCodeStrings[ConditionCode], StatusRegister, Fields,
					   SecondOperandString);
			}
			else if (REV_CONDITION(InstructionEncoding, SecondOpcode) ||
					 REV16_CONDITION(InstructionEncoding, SecondOpcode) ||
					 REVSH_CONDITION(InstructionEncoding, SecondOpcode))
			{
				if (REV_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "rev";
				}
				else if (REV16_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "rev16";
				}
				else if (REVSH_CONDITION(InstructionEncoding, SecondOpcode))
				{
					InstructionMnemonic = "revsh";
				}
				else
				{
					Stopif(true, return 1, "Bad rev instruction");
				}
				ComputeSecondDestination(&SecondOperandRegister, &DestinationRegister, NextInstruction);
				printf("%s%s r%d, r%d\n", InstructionMnemonic, ConditionCodeStrings[ConditionCode],
					   DestinationRegister, SecondOperandRegister);
			}
			else if (GET_BITS(NextInstruction, 26, 27) == 0x1)
			{
				uint32 PackHalfByteOpCode = GET_BITS(NextInstruction, 4, 6);
				bool32 PackHalfByteBottomTop = (PackHalfByteOpCode == 0x1);
				if ((InstructionEncoding == 0x68) &&
					(PackHalfByteBottomTop || (PackHalfByteOpCode == 0x5)))
				{
					ComputeFirstSecond(&FirstOperandRegister, &SecondOperandRegister, NextInstruction);
					InstructionMnemonic = PackHalfByteBottomTop ? "pkhbt" : "pkhtb";
					char *ShiftString = PackHalfByteBottomTop ? "LSL" : "ASR";
					printf("%s%s r%d, r%d, r%d, %s #%d\n", InstructionMnemonic,
						   ConditionCodeStrings[ConditionCode], GET_BITS(NextInstruction, 12, 15),
						   FirstOperandRegister, SecondOperandRegister, ShiftString,
						   GET_BITS(NextInstruction, 7, 11));
				}
				else if ((ConditionCode == 0xF) && ISOLATE_BIT(NextInstruction, 24) &&
						 (GET_BITS(NextInstruction, 20, 22) == 0x5) && (GET_BITS(NextInstruction, 12, 15) == 0xF))
				{
					FirstOperandRegister = GET_BITS(NextInstruction, 16, 19);
					char OffsetString[MAX_OFFSET_STRING_LENGTH];
					ComputeAddressingModeTwoOffset(OffsetString, NextInstruction);
					printf("pld [r%d, %s]\n", FirstOperandRegister, OffsetString);
				}
				else
				{
					InstructionMnemonic = GetLoadStoreString(NextInstruction);
					ComputeFirstDestination(&FirstOperandRegister, &DestinationRegister, NextInstruction);
					char *Byte = ISOLATE_BIT(NextInstruction, 22) ? "b" : "";
					char *PreIndexedString = ""; 
					char *Unprivileged = "";
					char FirstWord[MAX_INSTRUCTION_MNEMONIC_LENGTH];
					if (ISOLATE_BIT(NextInstruction, 21))
					{
						if (ISOLATE_BIT(NextInstruction, 24))
						{
							PreIndexedString = "!";
						}
						else
						{
							Unprivileged = "t";
						}
					}
					snprintf(FirstWord, MAX_INSTRUCTION_MNEMONIC_LENGTH, "%s%s%s%s",
							 InstructionMnemonic, ConditionCodeStrings[ConditionCode], Byte, Unprivileged);
					char OffsetString[MAX_OFFSET_STRING_LENGTH];
					ComputeAddressingModeTwoOffset(OffsetString, NextInstruction);
					bool32 PreIndexed = ISOLATE_BIT(NextInstruction, 24);
					if (PreIndexed)
					{
						printf("%s r%d, [r%d, %s]%s\n", FirstWord, DestinationRegister,
							   FirstOperandRegister, OffsetString, PreIndexedString);
					}
					else
					{
						printf("%s r%d, [r%d], %s\n", FirstWord, DestinationRegister,
							   FirstOperandRegister, OffsetString);
					}
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
					case EOR:
					case EOR_IMMEDIATE:
					case EOR_S:
					case EOR_IMMEDIATE_S:
					case ORR:
					case ORR_S:
					case ORR_IMMEDIATE:
					case ORR_IMMEDIATE_S:
					case RSB:
					case RSB_IMMEDIATE:
					case RSB_S:
					case RSB_IMMEDIATE_S:
					case RSC:
					case RSC_IMMEDIATE:
					case RSC_S:
					case RSC_IMMEDIATE_S:
					case SBC:
					case SBC_IMMEDIATE:
					case SBC_S:
					case SBC_IMMEDIATE_S:
                    {
                        Stopif(~ISOLATE_BIT(NextInstruction, 25) & ISOLATE_BIT(NextInstruction, 4) &
                               ISOLATE_BIT(NextInstruction, 7),
                               return 1,
                               "Load/Store instruction extension space");
						ComputeFirstDestination(&FirstOperandRegister, &DestinationRegister, NextInstruction);
						char SecondOperandString[MAX_OFFSET_STRING_LENGTH];
						ComputeSecondOperandString(SecondOperandString, NextInstruction);
						printf("%s%s r%d, r%d, %s\n", EncodingToMnemonic(InstructionEncoding),
							   ConditionCodeStrings[ConditionCode], DestinationRegister,
							   FirstOperandRegister, SecondOperandString);
                        break;
                    }
					case MOV:
					case MOV_S:
                    case MOV_IMMEDIATE:
                    case MOV_IMMEDIATE_S:
					case MVN:
					case MVN_S:
					case MVN_IMMEDIATE:
					case MVN_IMMEDIATE_S:
					{
                        uint32 DestinationRegister = (NextInstruction >> 12) & 0xF;
						char SecondOperandString[MAX_OFFSET_STRING_LENGTH];
						ComputeSecondOperandString(SecondOperandString, NextInstruction);
						printf("%s%s r%d, %s\n", EncodingToMnemonic(InstructionEncoding),
							   ConditionCodeStrings[ConditionCode], DestinationRegister,
							   SecondOperandString);
						break;
					}
                    case COMPARE_NEG:
                    case COMPARE_NEG_IMMEDIATE:
					case COMPARE:
					case COMPARE_IMMEDIATE:
					{
                        FirstOperandRegister = (NextInstruction >> 16) & 0xF;
						char SecondOperandString[MAX_OFFSET_STRING_LENGTH];
						ComputeSecondOperandString(SecondOperandString, NextInstruction);
						printf("%s%s r%d, %s\n", EncodingToMnemonic(InstructionEncoding),
							   ConditionCodeStrings[ConditionCode], FirstOperandRegister,
							   SecondOperandString);
						break;
					}
                    case COUNT_LEADING_ZEROS:
                    {
                        if (SecondOpcode == 0x1)
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
					case MCRR:
					case MRRC:
					{
						uint32 FirstArmRegister = GET_BITS(NextInstruction, 12, 15);
						uint32 SecondArmRegister = GET_BITS(NextInstruction, 16, 19);
						uint32 Coprocessor = GET_BITS(NextInstruction, 8, 11);
						uint32 DestinationCpReg = NextInstruction & 0xF;
						InstructionMnemonic = (InstructionEncoding == MCRR) ? "mcrr" : "mrrc";
						if (ConditionCode == 0xF)
						{
							printf("%s2 p%d, %d, r%d, r%d, c%d\n", InstructionMnemonic, Coprocessor, SecondOpcode,
								   FirstArmRegister, SecondArmRegister, DestinationCpReg);
						}
						else
						{
							printf("%s%s p%d, %d, r%d, r%d, c%d\n", InstructionMnemonic,
								   ConditionCodeStrings[ConditionCode], Coprocessor, SecondOpcode,
								   FirstArmRegister, SecondArmRegister, DestinationCpReg);
						}
						break;
					}
                    case MRS_SPSR:
                    case MRS_CPSR:
                    {
                        uint32 RegisterIndex = GET_BITS(NextInstruction, 12, 15);
						char *StatusRegister = ISOLATE_BIT(NextInstruction, 22) ? "spsr" : "cpsr";
                        printf("mrs%s r%d, %s\n", ConditionCodeStrings[ConditionCode], RegisterIndex,
							   StatusRegister);
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
