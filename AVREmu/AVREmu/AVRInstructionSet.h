#include "Plugin.h"
#include "emulator.h"
#include <string>

class AVRInstructionSet : public Plugin {
public:
	enum reg { // named access to registers/register pairs
		r0 = 0x00, r1 = 0x01, r2 = 0x02, r3 = 0x03, r4 = 0x04, r5 = 0x05, r6 = 0x06, r7 = 0x07,
		r8 = 0x08, r9 = 0x09, r10 = 0x0a, r11 = 0x0b, r12 = 0x0c, r13 = 0x0d, r14 = 0x0e, r15 = 0x0f,
		r16 = 0x10, r17 = 0x11, r18 = 0x12, r19 = 0x13, r20 = 0x14, r21 = 0x15, r22 = 0x16, r23 = 0x17,
		r24 = 0x18, r25 = 0x19, r26 = 0x1a, r27 = 0x1b, r28 = 0x1c, r29 = 0x1d, r30 = 0x1e, r31 = 0x1f,
		XL = 0x1a, XH = 0x1b, rX = 0x1a, YL = 0x1c, YH = 0x1d, rY = 0x1c, ZL = 0x1e, ZH = 0x1f, rZ = 0x1e,
		SPL = 0x5D, SPH = 0x5E,
		SREG = 0x5F
	};

	enum Sreg { carry, zero, neg, overflow, sign, halfcarry, copystorage, interrupt };

	enum offsets {
		// offsets for D/R
		d5bit = 0x01F0, d4bit = 0x00F0, d3bit = 0x0070,
		r5bith = 0x0200, r4bit = 0x000F, r3bit = 0x0007,
		kH = 0x0F00, kL = 0x000F,
		s3bit = 0x000E,
	};

	enum opcodes {
		NOP, MOVW, MULS, MULSU, FMUL, FMULS, FMULSU, CPC, SBC, ADD, CP, SUB, ADC, CPSE, AND,
		EOR, OR, MOV, CPI, SBCI, SUBI, ORI, SBR, ANDI, CBR, LDDZpk, STDZpk, LDDYpk, STDYpk, LDSI, STSI,
		COM, LDZp, STZp, NEG, LDmZ, STmZ, SWAP, INC, LMPZ, XCH, LPMZp, LAS, ASR, ELMPZ, LAC, LSR,
		ELPMZp, LAT, ROR, LDYp, STYp, LDmY, STmY, LDX, STX, LDXp, STXp, LDmX, STmX, PUSH, POP,
		JMPABS22, CLLABS22, BSET, BCLR, SEC, CLC, SEN, CLN, SEZ, CLZ, SEI, CLI, SES, CLS, SEV,
		CLV, SET, CLT, SEH, CLH, RET, RETI, SLEEP, BREAK, WDR, LPM, ELPM, SPM, SPMZp, IJMPZ, EIJMPZ,
		ICLLZ, EICLLZ, DEC, DES, ADIW, SBIW, CBI, SBIC, SBI, SBIS, MUL, IN, OUT, RJMP, RCLL, LDI,
		SER, BLD, BST, SBRS, SBRC, BRBS, BRBC, BREQ, BRNE, BRCS, BRCC, BRSH, BRLO, BRMI, BRPL,
		BRGE, BRLT, BRHS, BRHC, BRTS, BRTC, BRVS, BRVC, BRIE, BRID, TST, CLR,

		MAXINSTR, HLT = 0xFFFF
		
		/*NOP = 0x0000, HLT = 0xFFFF,
		LDST_INDIRECT = 0x9000, LDST_INDIRECT_MASK = 0xFC00, LDST_INDIRECT_OP_MASK = 0x000F,
		LDST_INDIRECT_SECONDARY = 0x0204, LDST_INDIRECT_SECONDARY_MASK = 0x020C,
		LDSSTS_I = 0, LDST_ZP = 1, LDST_YP = 9, LDST_DZ = 2, LDST_DY = 10,
		LPM_Z = 4, ELPM_Z = 6, LPM_ZP = 5, ELPM_ZP = 7,
		LDST_X = 12, LDST_XP = 13, LDST_DX = 14, POP_PUSH = 15,
		XCH = 4, LAS = 5, LAC = 6, LAT = 7,

		MISC = 0x9408, MISC_MASK = 0xFC08, MISC_OP_MASK = 0x00F0, JMP_OP_MASK = 0x0007,
		RET = 0, RETI = 1, SLEEP = 8, BREAK = 9, WDR = 10, LPM = 12,
		ELPM = 13, SPM = 14, SPM_ZP = 15,
		INDZ = 1, EINDZ = 1, DEC = 2, DES = 3, JMPABS = 4, JMPABS2 = 5,
		CALLABS = 6, CALLABS2 = 7,

		UNARY = 0x9400, UNARY_MASK = 0xFC08, UNARY_OP_MASK = 0x0007,
		COM = 0, NEG = 1, SWAP = 2, INC = 3, ASR = 5, LSR = 6, ROR = 7,

		MUL_ETC = 0x0000, MUL_ETC_MASK = 0xFC00, MUL_ETC_OP_MASK = 0x0300,
		MOVW = 1, MULS = 2, MULSU = 3, FMUL = 3, FMULS = 3, FMULSU = 3,

		ADD_ETC = 0x0000, ADD_ETC_MASK = 0xC000, ADD_ETC_OP_MASK = 0x3C00,
		CP = 5, CPC = 1, SUB = 6, SBC = 2, ADD = 3, ADC = 7, // ROL/LSL are aliases
		CPSE = 4, AND = 8, EOR = 9, OR = 10, MOV = 11,

		K_SERIES = 0x0000, K_SERIES_MASK = 0x8000, K_SERIES_OP_MASK = 0x7000,
		CPI = 3, SBCI = 4, SUBI = 5, ORI = 6, SBR = 6, ANDI = 7, CBR = 7,*/
	};

	//AVRInstructionSet();

	//std::vector<void *> ops;

	void buildNewSreg8();
	void buildNewSreg16();

	void applyState();
	void generateState();

private:
	// common working vars - may be used by ANY operation, should be considered
	// transient after a successful isFree()

	word d = 0x0000, r = 0x0000;// destination/source register addresses (plus extended addresses)
	byte opd = 0x00, opr = 0x00;// Contents of operand registers
	byte k = 0x0000;			// Immediate data for add/sub/compare/offsets for Z+k etc.
	byte selector = 0x00;		// Subsidiary opcode identification 
	word addr = 0x0000;			// Immediate address within data space
	dword absaddr;				// 22-bit absolute address
	byte result8;				// 8-bit storage for results
	word result16;				// 16-bit storage for results

	byte reg = 0x00;			// updated register contents for working

	byte sreg = 0xFF;			// new sreg; ANDed with old after instructions.
								//Bits stay set unless explicitly cleared.

	std::string opstring = "Unknown opcode"; // overwritten by recognised ops
};