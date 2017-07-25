
#include <iostream>
#include "avrinstructionset.h"

void AVRInstructionSet::buildNewSreg8() {
	// C: (Rd7 AND Rr7 OR Rr7 AND !R7 or !R7 AND !R0)
	if (
		((opd & (1 << 7)) && (opr & (1 << 7))) ||
		((opr & (1 << 7)) && !(resultByte & (1 << 7))) ||
		(!(resultByte & (1 << 7)) && !(resultByte & (1 << 0)))
	) sreg |= (1 << carry);
	else sreg &= ~(1 << carry);

	// Z: (!R7:1)
	if (resultByte & 0xFF) sreg |= (1 << zero);
	else sreg &= ~(1 << zero);

	// N: (R7)
	if (resultByte & 0x80) sreg |= (1 << neg);
	else sreg &= ~(1 << neg);

	// V: (Rd7 AND Rr7 AND !R7 OR !Rd7 AND Rr7 AND R7)
	if (
		(opd & (1 << 7)) && (opr & (1 << 7)) && (~resultByte & (1 << 7)) ||
		(~opd & (1 << 7)) && (~opr & (1 << 7)) && (resultByte & (1 << 7))
	) sreg |= (1 << overflow);
	else sreg &= ~(1 << overflow);
	
	// S: sign is always XOR neg/overflow.
	sreg &= ~(1 << sign); // clear bit
	sreg |= (((sreg >> neg) ^ (sreg >> overflow)) & 0x01) << sign; // set result
	
	// H: (Rd3 AND Rr3 OR Rr3 AND !R3 OR !R3 AND Rd3)
	if (
		((opd & (1 << 3)) && (opr & (1 << 3))) ||
		((opr & (1 << 3)) && (~resultByte & (1 << 3))) ||
		((~resultByte & (1 << 3)) && (opd & (1 << 3)))
	) sreg |= (1 << halfcarry);
	else sreg &= ~(1 << halfcarry);
}

void AVRInstructionSet::buildNewSreg16() {
	// C: (Rd7 AND Rr7 OR Rr7 AND !R7 or !R7 AND !R0)
	if (
		((opd & (1 << 7)) && (opr & (1 << 7))) ||
		((opr & (1 << 7)) && !(resultByte & (1 << 7))) ||
		(!(resultByte & (1 << 7)) && !(resultByte & (1 << 0)))
		) sreg |= (1 << carry);
	else sreg &= ~(1 << carry);

	// Z: (!R7:1)
	if (resultByte & 0xFF) sreg |= (1 << zero);
	else sreg &= ~(1 << zero);

	// N: (R7)
	if (resultByte & 0x80) sreg |= (1 << neg);
	else sreg &= ~(1 << neg);

	// V: (Rd7 AND Rr7 AND !R7 OR !Rd7 AND Rr7 AND R7)
	if (
		(opd & (1 << 7)) && (opr & (1 << 7)) && (~resultByte & (1 << 7)) ||
		(~opd & (1 << 7)) && (~opr & (1 << 7)) && (resultByte & (1 << 7))
		) sreg |= (1 << overflow);
	else sreg &= ~(1 << overflow);

	// S: sign is always XOR neg/overflow.
	sreg &= ~(1 << sign); // clear bit
	sreg |= (((sreg >> neg) ^ (sreg >> overflow)) & 0x01) << sign; // set result

	// H: (Rd3 AND Rr3 OR Rr3 AND !R3 OR !R3 AND Rd3)
	if (
		((opd & (1 << 3)) && (opr & (1 << 3))) ||
		((opr & (1 << 3)) && (~resultByte & (1 << 3))) ||
		((~resultByte & (1 << 3)) && (opd & (1 << 3)))
		) sreg |= (1 << halfcarry);
	else sreg &= ~(1 << halfcarry);

}

void AVRInstructionSet::operands3bit(word * d, word * r, word inst) {
	*d = (inst & d3bit) >> 4;
	*r = inst & r3bit;
}

void AVRInstructionSet::operands4bit(word * d, word * r, word inst) {
	*d = (inst & d4bit) >> 4;
	*r = inst & r4bit;
}

void AVRInstructionSet::operands5bit(word * d, word * r, word inst) {
	*d = (inst & d5bit) >> 4;
	*r = ((inst & r5bith) >> 5) | (inst & r4bit);
}

void AVRInstructionSet::operand5bit(word * d, word inst) {
	*d = (inst & d5bit) >> 4;
}

void AVRInstructionSet::operand24bit(word * d, word inst) {

}

void AVRInstructionSet::operands4bitAndConstant(word * d, word * k, word inst) {
	*d = (inst & d4bit) >> 4;
	*k = ((inst & kH) >> 4) | (inst & kL);
}

void AVRInstructionSet::operands5bitAnd3bit(word * d, word * k, word inst) {
	*d = (inst & 0xF8 >> 3);
	*k = inst & r3bit;
}

void AVRInstructionSet::operands6bitAnd3bit(word * d, word * k, word inst) {
	*d = (inst & 0x03F8 >> 3);
	*k = inst & r3bit;
}

void AVRInstructionSet::ioOperands(word * d, word * r, word inst) {
	*d = (inst & d5bit) >> 4;
	*r = ((inst & 0x0600) >> 5) | (inst & r4bit);
}

void AVRInstructionSet::generateState() {
	retrieve();
	host->pc += host->instructionSize;
}

int AVRInstructionSet::retrieve() {

	int wordCount = 1;

		try {

			// retrieve
			if (host->getConfig(Emulator::VERBOSE))
				log("Reading instruction 0x" + std::to_string(host->pc));

			inst = host->readWord(host->pc);

			//decode

			low = inst & 0xFF;
			high = inst >> 8;

			nib0 = low & 0xF;
			nib1 = (low & 0xF0) >> 4;
			nib2 = high & 0xF;
			nib3 = (high & 0xF0) >> 4;

			if (inst == NOP) operation = NOP;
			else if (inst == HLT) operation = HLT;

			else {
				switch (nib3) {

				// ### Groups defined by nib3 ONLY

				case 0x03:
					operation = CPI;
					break;

				case 0x04:
					operation = SBCI;
					break;

				case 0x05:
					operation = SUBI;
					break;

				case 0x06: // ORI/SBR
					operation = ORI;
					break;

				case 0x07: // ANDI/CBR
					operation = ANDI;
					break;

				// Groups defined by nib3 AND nib2

				case 0x00: // MOVW/MULS family
					switch (nib2) {
					case 0x01:
						operation = MOVW;
						break;
					case 0x02:
						operation = MULS;
						break;
					case 0x03:
						if (nib0 <= 7 && nib1 <= 7) {
							operation = MULSU;
							commit(2);
						}
						else if (nib0 <= 7 && nib1 > 7)	{
							operation = FMUL;
							commit(2);
						}
						else if (nib0 > 7 && nib1 <= 7)	{
							operation = FMULS;
							commit(2);
						}
						else {
							operation = FMULSU;
							commit(2);
						}
						break;
					case 0x08:
						operation = MULS;
						break;
					default:
						if (nib2 <= 0x07) operation = CPC;
						else if (nib2 <= 0x0B) operation = SBC;
						else operation = ADD;
					}
					break;

				case 0x01: // CPSE/CP/SUB family
					if (nib2 <= 0x03) operation = CPSE;
					else if (nib2 <= 0x07) operation = CP;
					else if (nib2 == 0x0B) operation = SUB;
					else operation = ADC;
					break;

				case 0x02: // AND/EOR/OR/MOV family
					if (nib2 <= 0x03) operation = AND;
					else if (nib2 <= 0x07) operation = EOR;
					else if (nib2 <= 0x0B) operation = OR;
					else operation = MOV;
					break;


				case 0x08: // LDD/STD Z+k/Y+k
				case 0x0A:
					if ((nib2 & 0x2) == 0) { // LDD
						if ((nib0 & 0x8) == 0) operation = LDDZpk;
						else operation = LDDYpk;
					}
					else { // STD
						if ((nib0 & 0x8) == 0) operation = STDZpk;
						else operation = STDYpk;

					}

					commit(2);
					break;

				case 0x09:
					if (nib2 <= 5){
						// patterns of 0-1, 2-3, 4-5 can be condensed to 0, 1, 2 by nib2/2
						selector = nib2 / 2;

						switch (nib0) {
						case 0x00:
							switch (selector) { // TODO
							case 0:
								operation = LDS;
								commit(2);
								wordCount = 2;
								break;
							case 1:
								operation = STS;
								commit(2);
								wordCount = 2;
								break;
							case 2:
								operation = COM;
								break;
							}
							break;

						case 0x01:
							switch (selector) { // TODO
							case 0:
								operation = LDZp;
								break;
							case 1:
								operation = STZp;
								break;
							case 2:
								operation = NEG;
								break;
							}
							break;

						case 0x02:
							switch (selector) { // TODO
							case 0:
								operation = LDmZ;
								break;
							case 1:
								operation = STmZ;
								break;
							case 2:
								operation = SWAP;
								break;
							}
							break;

						case 0x03:
							operation = INC;
							break;
						case 0x04:
							switch (selector) { // TODO
							case 0:
								operation = LMPZ;
								break;
							case 1:
								operation = XCH;
								commit(2);
								break;
							}
							break;

						case 0x05:
							switch (selector) { // TODO
							case 0:
								operation = LPMZp;
								break;
							case 1:
								operation = LAS;
								commit(2);
								break;
							case 2:
								operation = ASR;
								break;
							}
							break;
						case 0x06:
							switch (selector) { // TODO
							case 0:
								operation = ELMPZ;
								break;
							case 1:
								operation = LAC;
								commit(2);
								break;
							case 2:
								operation = LSR;
								break;
							}
							break;

						case 0x07:
							switch (selector) { // TODO
							case 0:
								operation = ELPMZp;
								break;
							case 1:
								operation = LAT;
								commit(2);
								break;
							case 2:
								operation = ROR;
								break;
							}
							break;

						case 0x08:
							if (nib2 == 0x04) {
								d = (nib1 & 0x7) >> 1;

								if (nib1 <= 0x07) operation = BSET;
								else operation = BCLR;
							}
							else if (nib2 == 0x05) {
								switch (nib1) {
								case 0x00:
									operation = RET;
									commit(4);
									break;
								case 0x01:
									operation = RETI;
									commit(4);
									break;
								case 0x08:
									operation = SLEEP;
									break;
								case 0x09:
									operation = BREAK;
									break;
								case 0x0A:
									operation = WDR;
									break;
								case 0x0C:
									operation = LPM;
									break;
								case 0x0D:
									operation = ELPM;
									commit(3);
									break;
								case 0x0E:
									operation = SPM;
									break;
								case 0x0F:
									operation = SPMZp;
									break;
								}
							}
							break;

						case 0x09:
							switch (nib2) {
							case 4:
								if (nib1 == 0) {
									operation = IJMPZ;
									commit(2);
								}
								else {
									operation = EIJMPZ;
								}
								break;
							case 5:
								if (nib1 == 0) {
									operation = ICALL;
									commit(3);
								}
								else {
									operation = EICALL;
									commit(4);
								}
								break;
							}
							break;

						case 0x0A:
							if (nib2 <= 1) {
								operation = LDmY;
							}
							else {
								operation = LDYp;
							}

							break;

						case 0x0B:
							break;

						case 0x0C:
							if (nib2 <= 1) {
								operation = LDX;
								commit(2);
							}
							else if (nib2 <= 3) {
								operation = STX;
								commit(2);
							}
							else {
								operation = JMP;
								commit(3);
							}

							break;

						case 0x0D:
							if (nib2 <= 1) {
								operation = LDXp;
								commit(2);
							}
							else if (nib2 <= 3) {
								operation = STXp;
								commit(2);
							}
							else {
								operation = JMP;
								commit(3);
							}
							break;

						case 0x0E:
							if (nib2 <= 1) {
								operation = LDmX;
								commit(2);
							}
							else if (nib2 <= 3) {
								operation = STmX;
								commit(2);
							}
							else {
								operation = CALL;
								commit(4);
							}
							break;

						case 0x0F:
							if (nib2 <= 1) {
								operation = PUSH;
								commit(2);
							}
							else if (nib2 <= 3) {
								operation = POP;
								commit(2);
							}
							else {
								operation = CALL;
								commit(4);
							}
							break;
						}
						break;
					}
					else {
						switch (nib2) {
						case 0x06:
							operation = ADIW;
							break;
						case 0x07:
							operation = SBIW;
							commit(2);
							break;
						case 0x08:
							operation = CBI;
							commit(2);
							break;
						case 0x09:
							operation = SBC;
							break;
						case 0x0A:
							operation = SBI;
							commit(2);
							break;
						case 0x0B:
							operation = SBIS;
							break;
						case 0x0C:
						case 0x0D:
						case 0x0E:
						case 0x0F:
							operation = MUL;
							break;
						}
					}

				case 0x0B:
					if (nib2 <= 0x07) operation = IN;
					else operation = OUT;
					break;

				case 0x0C:
					operation = RJMP;
					commit(2);
					break;

				case 0x0D:
					operation = RCALL;
					commit(3);
					break;

				case 0x0E:
					operation = LDI;
					if (inst & 0x0F0F) operation = SER;
					break;

				case 0x0F:
					if (nib2 <= 3) operation = BRBS;
					else operation = BRBC;
					break;
				}
			}

			// TODO:
			// STX: commit(2);
			// LDX: commit(2);

		}
		catch (const char* except) {
			log(except);
		}

		return wordCount;
}

void AVRInstructionSet::applyState() {
	switch (operation) {
	case NOP: // 1 cycle

		opstring = "NOP";

		break;

	case MOVW: // 1 cycle
		operands4bit(&d, &r, inst);

		host->writeWord(d, host->readWord(r));

		opstring = "MOVW " + std::to_string(d) + ", " + std::to_string(r);
		break;

	case MULS: // 1 cycle
		operands4bit(&d, &r, inst);

		resultWord = (int16_t)host->readByte(d) * (int16_t)host->readByte(r);
		host->writeWord(d, resultWord);

		// todo: SREG update C = R15, Z = (R == 0)
		opstring = "MULS " + std::to_string(d) + ", " + std::to_string(r);
		break;

	case MULSU: // 2 cycles
		operands3bit(&d, &r, inst);

		resultWord = (int16_t)host->readByte(d) * (uint16_t)host->readByte(r);
		host->writeWord(d, resultWord);

		// todo: SREG update C = R15, Z = (R == 0)
		opstring = "MULSU " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case FMUL:
		break;
	case FMULS:
		break;
	case FMULSU:
		break;
	case CPC:
		break;
	case SBC: // 1 cycle
		operands5bit(&d, &r, inst);

		resultByte = host->readByte(d) - host->readByte(r) - host->readBit(SREG, carry);
		host->writeByte(d, resultByte);

		// Todo: SREG updates:
		//	H = (!Rd3 AND Rr3) OR (Rr3 AND R3) OR (R3 AND !Rd3)
		//	S = N ^ V
		//	V = (Rd7 AND !Rr7 AND !R7) OR (!RD7 AND Rr7 AND R7)
		//  N = R7
		//	Z = (R == 0)
		//	C = (!Rd7 AND Rr7) + (Rr7 AND R7) OR (R7 AND !Rd7)

		opstring = "SBC " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case ADD:
		operands5bit(&d, &r, inst);

		opd = host->readByte(d);
		opr = host->readByte(r);
		resultByte = opd + opr;
		host->writeByte(d, resultByte);

		opstring = "ADD " + std::to_string(d) + ", " + std::to_string(r);
		break;
	case CP:
		break;
	case SUB:
		break;
	case ADC:
		break;
	case CPSE:
		operands5bit(&d, &r, inst);

		if (host->readByte(d) == host->readByte(r)) {
			host->pc += (host->instructionSize * retrieve());
			uncommit();
		}

		opstring = "CPSE " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case AND:
		// if r == d then TST
		break;
	case EOR:
		// if r == d then CLR

		break;
	case OR: // 1 cycle
		operands5bit(&d, &r, inst);

		resultByte = host->readWord(d) | host->readWord(r);
		host->writeByte(d, resultByte);

		// todo: SREG update, S = N^V V = 0 N = R7, Z = (R == 0)
		opstring = "OR " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case MOV: // 1 cycle
		operands5bit(&d, &r, inst);
		host->writeByte(d, host->readByte(r));

		opstring = "MOV " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case CPI:
		break;
	case SBCI: // 1 cycle
		operands4bitAndConstant(&d, &k, inst);

		resultByte = host->readByte(d) - k - host->readBit(SREG, carry);
		host->writeByte(d, resultByte);

		// Todo: SREG updates:
		//	H
		//	S
		//	V
		//	N
		//	Z
		//	C

		opstring = "SBCI " + std::to_string(d) + ", " + std::to_string(k);

		break;
	case SUBI:
		break;
	case ORI: // 1 cycle
		operands4bitAndConstant(&d, &k, inst);

		resultByte = host->readByte(d) | k;
		host->writeByte(d, resultByte);

		// Todo: SREG update, S = N^V, V = 0, N = R7, Z = (R == 0)
		opstring = "ORI " + std::to_string(d) + ", $" + std::to_string(k);
		break;
	case ANDI:
		operands4bitAndConstant(&d, &k, inst);

		// Complement K for CBR.
		resultByte = host->readByte(d) & k;
		host->writeByte(d, resultByte);

		// Todo: SREG update, 
		
		opstring = "ANDI " + std::to_string(d) + ", $" + std::to_string(k);
		break;
	case LDDZpk:

		addr = host->readWord(rZ) + k;
		host->writeByte(d, host->readByte(addr));
		
		opstring = "LDDZ " + std::to_string(d);
		if (k != 0) opstring += " +k " + std::to_string(k);

		break;
	case STDZpk:

		addr = host->readWord(rZ) + k;
		host->writeByte(addr, host->readByte(d));

		opstring = "STDZ " + std::to_string(d);
		if (k != 0) opstring += " +k " + std::to_string(k);

		break;
	case LDDYpk:
		addr = host->readWord(rY) + k;
		if (k != 0) opstring += "+k";
		break;
	case STDYpk:
		d = host->readWord(rY) + k;

		if (k != 0) opstring += "+k";
		break;
	case LDS:
		break;
	case STS:
		break;
	case COM:
		break;
	case LDZp:
		break;
	case STZp:
		break;
	case NEG: // 1 cycle
		operand5bit(&d, inst);
		resultByte = ~(host->readByte(d)) + 1;
		host->writeByte(d, resultByte);
		break;
	case LDmZ:
		break;
	case STmZ:
		break;
	case SWAP:
		break;
	case INC:
		break;
	case XCH:
		break;
	case LPMZp:
		break;
	case LAS:
		break;
	case ASR:
		break;
	case ELMPZ:
		break;
	case LAC:
		break;
	case LSR:
		break;
	case ELPMZp:
		break;
	case LAT:
		break;
	case ROR: // 1 cycle
		operand5bit(&d, inst);

		// Todo: Implement. R >> 1, C to b7, b0 to C

		// Todo: SREG update S = N^V, V = N^C (after), N = R7, Z = (R == 0), C = Rd0

		opstring = "ROR " + std::to_string(d);

		break;
	case LDYp:
		break;
	case STYp:
		break;
	case LDmY:
		break;
	case STmY:
		break;
	case LDX:
		break;
	case STX:
		break;
	case LDXp:
		break;
	case STXp:
		break;
	case LDmX:
		break;
	case STmX:
		break;
	case PUSH: // 2 cycles
		operand5bit(&d, inst);

		host->writeByte(host->readWord(SP), host->readByte(d));
		host->writeWord(SP, host->readWord(SP) - 1);

		opstring = "PUSH " + std::to_string(d);

		break;
	case POP: // 2 cycles
		operand5bit(&d, inst);

		host->writeWord(SP, host->readWord(SP) + 1);
		host->writeByte(host->readWord(SP), host->readByte(d));

		opstring = "POP " + std::to_string(d);

		break;
	case JMP:
		break;
	case CALL:
		break;
	case BSET:
		host->setBit(SREG, d);

		switch (nib1) {
		case 0x00:
			opstring = "SEC";
			break;
		case 0x01:
			opstring = "SEZ";
			break;
		case 0x02:
			opstring = "SEN";
			break;
		case 0x03:
			opstring = "SEV";
			break;
		case 0x04:
			opstring = "SES";
			break;
		case 0x05:
			opstring = "SEH";
			break;
		case 0x06:
			opstring = "SET";
			break;
		case 0x07:
			opstring = "SEI";
			break;
		}
		break;
	case BCLR:
		host->clearBit(SREG, d);
		switch (nib1) {
		case 0x08:
			opstring = "CLC";
			break;
		case 0x09:
			opstring = "CLZ";
			break;
		case 0x0A:
			opstring = "CLN";
			break;
		case 0x0B:
			opstring = "CLV";
			break;
		case 0x0C:
			opstring = "CLS";
			break;
		case 0x0D:
			opstring = "CLH";
			break;
		case 0x0E:
			opstring = "CLT";
			break;
		case 0x0F:
			opstring = "CLI";
			break;
		}
		break;
	case SEC:
		break;
	case CLC:
		break;
	case SEN:
		break;
	case CLN:
		break;
	case SEZ:
		break;
	case CLZ:
		break;
	case SEI:
		break;
	case CLI:
		break;
	case SES:
		break;
	case CLS:
		break;
	case SEV:
		break;
	case CLV:
		break;
	case SET:
		break;
	case CLT:
		break;
	case SEH:
		break;
	case CLH:
		break;
	case RET: // 4 cycles on 16bit PC, 5 on 22bit PC
		host->pc = host->readWord(SP) + 1;
		host->writeWord(host->readWord(SP), host->readWord(SP) + 2); // For 22bit this is 3 bytes rather than 2
		

		opstring = "RET";
		break;
	case RETI: // 4 cycles on 16bit PC, 5 on 22bit PC
		host->pc = host->readWord(SP) + 1;
		host->writeWord(host->readWord(SP), host->readWord(SP) + 2); // For 22bit this is 3 bytes rather than 2

		host->setBit(SREG, interrupt);

		opstring = "RETI";

		break;
	case SLEEP:
		break;
	case BREAK:
		break;
	case WDR:
		break;
	case LPM:
		break;
	case ELPM:
		break;
	case SPM:
		break;
	case SPMZp:
		break;
	case IJMPZ:
		addr = host->readWord(rZ);
		host->pc = addr;

		opstring = "IJMP > " + std::to_string(addr); 

		break;
	case EIJMPZ:
		//todo
		commit(2);
		break;
	case ICALL:
		host->pc = addr;
		break;
	case EICALL:
		//todo
		break;
	case DEC:
		break;
	case DES:
		break;
	case ADIW:
		break;
	case SBIW:
		break;
	case CBI:
		operands5bitAnd3bit(&d, &k, inst);

		host->clearBit(d, k);

		opstring = "CBI " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case SBIC:
		operands5bitAnd3bit(&d, &k, inst);

		if (!host->readBit(d, k)) {
			host->pc += (host->instructionSize * retrieve()); // check width to skip
			uncommit();
		}

		opstring = "SBIC " + std::to_string(d) + ", " + std::to_string(k);
		
		break;

	case SBI: // 2 cycles
		operands5bitAnd3bit(&d, &k, inst);

		host->setBit(d, k);

		opstring = "SBI " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case SBIS:
		operands5bitAnd3bit(&d, &k, inst);

		if (host->readBit(d, k)) {
			host->pc += (host->instructionSize * retrieve()); // check width to skip
			uncommit();
		}

		opstring = "SBIS " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case MUL:
		operands5bit(&d, &r, inst);

		resultWord = host->readByte(d) * host->readByte(r);
		host->writeWord(r0, resultWord);

		// Todo: Sreg updates. C = R15, z = (R == 0)
		opstring = "MUL " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case IN:
		break;
	case OUT: // 1 cycle
		ioOperands(&d, &r, inst);

		host->writeByte(d, host->readByte(r));

		opstring = "OUT " + std::to_string(d) + ", " + std::to_string(r);

		break;
	case RJMP: // 2 cycles
		operand24bit(&d, inst);
		
		host->pc = d;

		opstring = "RJMP " + std::to_string(d);

		break;
	case RCALL: // 3 cycles on 16bit PC, 4 cycles on 22bit PC. Assuming 16-bit.
		operand24bit(&d, inst);

		// Store return address:
		// Test: SP is 0x0F, push moves to 0x0D, 0x0E/0x0F are new contents.
		host->writeWord(host->readWord(SP) - 1, host->pc + (1 * host->instructionSize)); // Store return address...
		host->writeWord(host->readWord(SP), host->readWord(SP) - 2); // Move SP. For 22bit this is 3 bytes rather than 2

		// Call:
		host->pc = d;

		opstring = "RCALL " + std::to_string(d);

		break;
	case LDI:
		break;
	case SER:
		break;
	case BLD:
		break;
	case BST:
		break;
	case SBRS:
		break;
	case SBRC:
		break;
	case BRBS:
		// 111100kkkkkkksss
		operands6bitAnd3bit(&d, &r, inst);

		if (host->readBit(sreg, r)) {
			host->pc += (int16_t)d;
		}

		switch (r) {
		case 0x00:
			opstring = "BRCS";
			break;
		case 0x01:
			opstring = "BREQ";
			break;
		case 0x02:
			opstring = "BRMI";
			break;
		case 0x03:
			opstring = "BRVS";
			break;
		case 0x04:
			opstring = "BRLT";
			break;
		case 0x05:
			opstring = "BRHS";
			break;
		case 0x06:
			opstring = "BRTS";
			break;
		case 0x07:
			opstring = "BRIE";
			break;
		default:
			opstring = "BRBS";
			break;
		}

		break;
	case BRBC:
		// 111101kkkkkkksss
		operands6bitAnd3bit(&d, &r, inst);

		if (!host->readBit(sreg, r)) {
			host->pc += (int16_t)d;
		}

		switch (r) {
		case 0x00:
			opstring = "BRCC";
			break;
		case 0x01:
			opstring = "BRNE";
			break;
		case 0x02:
			opstring = "BRPL";
			break;
		case 0x03:
			opstring = "BRVC";
			break;
		case 0x04:
			opstring = "BRGE";
			break;
		case 0x05:
			opstring = "BRHC";
			break;
		case 0x06:
			opstring = "BRTT";
			break;
		case 0x07:
			opstring = "BRID";
			break;
		default:
			opstring = "BRBC";
			break;
		}

		break;
	case HLT:
		host->setState(Emulator::STOP);
		opstring = "HLT";
		break;
	default:
		break;
	}

	host->writeByte(SREG, sreg); // Replace SREG with new sreg

	if (host->getConfig(Emulator::VERBOSE)) {
		log("-> " + opstring);
	}

	//if (mode == rWord) host->writeWord(d, resultWord);
	//else host->writeByte(d, resultByte);

	// reset sentinel values for transient stuff
	sreg = 0xFF;
	opstring = "Unknown opcode"; // overwritten by recognised ops
	// End TODO.
}

