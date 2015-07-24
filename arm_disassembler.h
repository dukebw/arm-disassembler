#ifndef _ARM_DISASSEMBLER_H_
#define _ARM_DISASSEMBLER_H_

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
#define AND 0x0
#define AND_S 0x1
#define AND_IMMEDIATE 0x20
#define AND_IMMEDIATE_S 0x21
#define BIC 0x1C
#define BIC_S 0x1D
#define BIC_IMMEDIATE 0x3C
#define BIC_IMMEDIATE_S 0x3D
#define BRANCH_EXCHANGE 0x12
#define BRANCH 0xA
#define BRANCH_LINK 0xB
#define COMPARE_NEG 0x17
#define COMPARE_NEG_IMMEDIATE 0x37
#define COUNT_LEADING_ZEROS 0x16
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
#define SHIFT_TYPE_IMMEDIATE(Instruction) GET_BITS((Instruction), 4, 6)
#define SHIFT_TYPE_REGISTER(Instruction) GET_BITS((Instruction), 4, 7)
#define SHIFT_TYPE_LSL_IMMEDIATE 0
#define SHIFT_TYPE_LSR_IMMEDIATE 2
#define SHIFT_TYPE_ASR_IMMEDIATE 4
#define SHIFT_TYPE_ROR_IMMEDIATE 6
#define SHIFT_TYPE_LSL_REGISTER 1
#define SHIFT_TYPE_LSR_REGISTER 3
#define SHIFT_TYPE_ASR_REGISTER 5
#define SHIFT_TYPE_ROR_REGISTER 7

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

#endif // _ARM_DISASSEMBLER_H_
