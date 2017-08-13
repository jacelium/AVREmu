#include "Plugin.h"
#include "emulator.h"
#include <string>
#include <sstream>

class AVRInstructionSet : public Plugin {
public:
	enum ResultMode {rWord, rByte};

	enum reg { // named access to registers/register pairs
		r0 = 0x00, r1 = 0x01, r2 = 0x02, r3 = 0x03, r4 = 0x04, r5 = 0x05, r6 = 0x06, r7 = 0x07,
		r8 = 0x08, r9 = 0x09, r10 = 0x0a, r11 = 0x0b, r12 = 0x0c, r13 = 0x0d, r14 = 0x0e, r15 = 0x0f,
		r16 = 0x10, r17 = 0x11, r18 = 0x12, r19 = 0x13, r20 = 0x14, r21 = 0x15, r22 = 0x16, r23 = 0x17,
		r24 = 0x18, r25 = 0x19, r26 = 0x1a, r27 = 0x1b, r28 = 0x1c, r29 = 0x1d, r30 = 0x1e, r31 = 0x1f,
		XL = 0x1a, XH = 0x1b, rX = 0x1a, YL = 0x1c, YH = 0x1d, rY = 0x1c, ZL = 0x1e, ZH = 0x1f, rZ = 0x1e,
		SP = 0x5D, SPL = 0x5D, SPH = 0x5E,
		SREG = 0x5F
	};

	enum Sreg { carry, zero, neg, overflow, sign, halfcarry, copystorage, interrupt };

	enum offsets {
		// offsets for D/R
		d5bit = 0x01F0, d4bit = 0x00F0, d3bit = 0x0070,
		r5bith = 0x0200, r4bit = 0x000F, r3bit = 0x0007,
		kH = 0x0F00, kL = 0x000F,
		s3bit = 0x000E,
		MMIO_OFFSET = 0x20
	};

	enum Op {
		ADD, ADC, ADIW, SUB, SUBI, SBC, SBCI, SBIW, AND, ANDI,
		OR, ORI, EOR, COM, NEG, SBR, CBR, INC, DEC, TST, CLR,
		SER, MUL, MULS, MULSU, FMUL, FMULS, FMULSU,
		RJMP, IJMP, JMP, RCALL, ICALL, CALL, RET, RETI,
		CPSE, CP, CPC, CPI, SBRC, SBRS, SBIC, SBIS,
		BRBS, BRBC, BREQ, BRNE, BRCS, BRCC, BRSH, BRLO,
		BRMI, BRPL, BRGE, BRLT, BRHS, BRHC, BRTS, BRTC,
		BRVS, BRVC, BRIE, BRID, SBI, CBI, LSL, LSR, ROL,
		ROR, ASR, SWAP, BSET, BCLR, BST, BLD, SEC, CLC,
		SEN, CLN, SEZ, CLZ, SEI, CLI, SES, CLS, SEV, CLV,
		SET, CLT, SEH, CLH, MOV, MOVW, LDI, LDX, LDXp, LDmX,
		LDY, LDYp, LDmY, LDDY, LDZ, LDZp, LDmZ, LDDZ,
		LDS, STX, STXp, STmX, STY, STYp, STmY, STDY,
		STZ, STZp, STmZ, STDZ, STS, LPM, LPMp, SPM,
		IN, OUT, PUSH, POP, NOP, SLEEP, WDR, BREAK,

		MAXINSTR, UNKN
	};

	void buildNewSreg8();
	void buildNewSreg16();

	void applyState();
	void generateState();

	int retrieve();

private:


	// common working vars - may be used by ANY operation, should be considered
	// transient after applyState() call.

	word inst;					// Current instruction word
	byte low, high;				// Current high/low bytes
	byte nib0, nib1, nib2, nib3;// Current LL/LH/HL/HH nibbles
	word d = 0x0000, r = 0x0000;// destination/source register addresses (plus extended addresses)
	byte opd = 0x00, opr = 0x00;// Contents of operand registers
	word k = 0x0000;			// Immediate data for add/sub/compare/offsets for Z+k etc.
	byte selector = 0x00;		// Subsidiary opcode identification 
	word addr = 0x0000;			// Immediate address within data space
	dword absaddr;				// 22-bit absolute address
	byte resultByte;			// 8-bit storage for results
	word resultWord;			// 16-bit storage for results

	byte reg = 0x00;			// updated register contents for working

	byte sreg = 0xFF;			// new sreg; ANDed with old after instructions.
								//Bits stay set unless explicitly cleared.

	std::stringstream opstring; // overwritten by recognised ops
	std::stringstream msg;

	Op operation;
	ResultMode mode;
	void operands3bit(word * d, word * r, word inst);
	void operands4bit(word * d, word * r, word inst);
	void operands5bit(word * d, word * r, word inst);
	void operand5bit(word * d, word inst);
	void operand22bit(dword * addr, word inst, word inst2);

	void operands4bitAndConstant(word * d, word * k, word inst);
	void operands5bitAnd3bit(word * d, word * k, word inst);
	void operands6bitAnd3bit(word * d, word * k, word inst);
	void ioOperands(word * d, word * r, word inst);
	void iwOperands(word * d, word * r, word inst);

	std::string name();
};
