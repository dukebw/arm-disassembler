#ifndef _ARM_DISASSEMBLER_H_
#define _ARM_DISASSEMBLER_H_

#define ARM_INSTRUCTION_LENGTH				4
#define THUMB_INSTRUCTION_LENGTH			2

#define ADC_IMMEDIATE						0x2A
#define ADC_IMMEDIATE_S						0x2B
#define ADC									0xA
#define ADC_S								0xB
#define ADD_IMMEDIATE						0x28
#define ADD_IMMEDIATE_S						0x29
#define ADD									0x8
#define ADD_S								0x9
#define AND									0x0
#define AND_S								0x1
#define AND_IMMEDIATE						0x20
#define AND_IMMEDIATE_S						0x21
#define BIC									0x1C
#define BIC_S								0x1D
#define BIC_IMMEDIATE						0x3C
#define BIC_IMMEDIATE_S						0x3D
#define BRANCH_EXCHANGE						0x12
#define BRANCH								0xA
#define BRANCH_LINK							0xB
#define COUNT_LEADING_ZEROS 				0x16
#define COMPARE_NEG							0x17
#define COMPARE_NEG_IMMEDIATE				0x37
#define COMPARE								0x15
#define COMPARE_IMMEDIATE					0x35
#define COPY								0x1A
#define EOR									0x2
#define EOR_IMMEDIATE						0x22
#define EOR_S								0x3
#define EOR_IMMEDIATE_S						0x23
#define LOAD_IMMEDIATE						0x59
#define MCRR								0xC4
#define MRRC								0xC5
#define MOV									0x1A
#define MOV_S								0x1B
#define MOV_IMMEDIATE						0x3A
#define MOV_IMMEDIATE_S						0x3B
#define MRS_CPSR							0x10
#define MRS_SPSR							0x14
#define MSR_IMMEDIATE_CPSR					0x32
#define MSR_IMMEDIATE_SPSR					0x36
#define MSR_REGISTER_CPSR					0x12
#define MSR_REGISTER_SPSR					0x16
#define MVN									0x1E
#define MVN_S								0x1F
#define MVN_IMMEDIATE						0x3E
#define MVN_IMMEDIATE_S						0x3F
#define ORR									0x18
#define ORR_S								0x19
#define ORR_IMMEDIATE						0x38
#define ORR_IMMEDIATE_S						0x39

#define LINK_REGISTER						14
#define PROGRAM_COUNTER_REGISTER			15
#define ROM_SIZE							0x100000
#define GENERAL_PURPOSE_REG_COUNT			16

#define COND_EQ								0x0
#define COND_NE								0x1
#define COND_CS_HS							0x2
#define COND_CC_LO							0x3
#define COND_MI								0x4
#define COND_PL								0x5
#define COND_VS								0x6
#define COND_VC								0x7
#define COND_HI								0x8
#define COND_LS								0x9
#define COND_GE								0xA
#define COND_LT								0xB
#define COND_GT								0xC
#define COND_LE								0xD
#define COND_AL								0xE

#define MODE_USER							0x10
#define MODE_FIQ							0x11
#define MODE_IRQ							0x12
#define MODE_SUPERVISOR						0x13
#define MODE_ABORT							0x17
#define MODE_UNDEFINED						0x1B
#define MODE_SYSTEM							0x1F

#define N_BIT_INDEX							31
#define Z_BIT_INDEX 						30
#define C_BIT_INDEX 						29
#define V_BIT_INDEX 						28
#define Q_BIT_INDEX 						27
#define S_BIT_INDEX 						20
#define ARM_STATE_BIT_INDEX					5
#define DISABLE_FAST_INTERRUPTS_BIT_INDEX	6
#define DISABLE_NORMAL_INTERRUPTS_BIT_INDEX 7

#define MAX_INSTRUCTION_MNEMONIC_LENGTH		8
#define MAX_OFFSET_STRING_LENGTH			32
#define MSR_FIELDS_COUNT					4

// NOTE(brendan): data processing operand instructions
#define SHIFT_TYPE_IMMEDIATE(Instruction) GET_BITS((Instruction), 4, 6)
#define SHIFT_TYPE_REGISTER(Instruction) GET_BITS((Instruction), 4, 7)
#define SHIFT_TYPE_LSL_IMMEDIATE			0
#define SHIFT_TYPE_LSR_IMMEDIATE			2
#define SHIFT_TYPE_ASR_IMMEDIATE			4
#define SHIFT_TYPE_ROR_IMMEDIATE			6
#define SHIFT_TYPE_LSL_REGISTER				1
#define SHIFT_TYPE_LSR_REGISTER				3
#define SHIFT_TYPE_ASR_REGISTER				5
#define SHIFT_TYPE_ROR_REGISTER				7

#define GET_BITS(Register, StartBit, EndBit) \
    (((uint32)(Register) << (31u - (EndBit))) >> ((StartBit) + 31u - (EndBit)))
#define ISOLATE_BIT(Register, Bit) (((Register) >> (Bit)) & 1)
#define N_BIT(Register) ISOLATE_BIT((Register), N_BIT_INDEX)
#define Z_BIT(Register) ISOLATE_BIT((Register), Z_BIT_INDEX)
#define C_BIT(Register) ISOLATE_BIT((Register), C_BIT_INDEX)
#define V_BIT(Register) ISOLATE_BIT((Register), V_BIT_INDEX)
#define Q_BIT(Register) ISOLATE_BIT((Register), Q_BIT_INDEX)
#define S_BIT(Instruction) ISOLATE_BIT((Instruction), S_BIT_INDEX)

#define SET_BIT(Register, Value, BitToSet) (Register) |= (((Value) & 1) << (BitToSet))

#define QDADD_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x14) && ((SecondOpcode) == 0x5))
#define QADD_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x10) && ((SecondOpcode) == 0x5))
#define QADD16_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0x1))
#define QADD8_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0x9))
#define QADDSUBX_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0x3))
#define QDSUB_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x16) && ((SecondOpcode) == 0x5))
#define QSUB_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x12) && ((SecondOpcode) == 0x5))
#define QSUB16_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0x7))
#define QSUB8_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0xF))
#define QSUBADDX_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x62) && ((SecondOpcode) == 0x5))

#define REV_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x6B) && ((SecondOpcode) == 0x3))
#define REV16_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x6B) && ((SecondOpcode) == 0xB))
#define REVSH_CONDITION(InstructionEncoding, SecondOpcode) \
	(((InstructionEncoding) == 0x6F) && ((SecondOpcode) == 0xB))

#endif // _ARM_DISASSEMBLER_H_
