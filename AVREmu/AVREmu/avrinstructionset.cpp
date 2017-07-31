
#include <iostream>
#include "avrinstructionset.h"
#include <sstream>

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
	*d = (inst & 0xF8) >> 3;
	*k = inst & r3bit;
}

void AVRInstructionSet::operands6bitAnd3bit(word * d, word * k, word inst) {
	*d = (inst & 0x03F8) >> 3;
	*k = inst & r3bit;
}

void AVRInstructionSet::ioOperands(word * d, word * r, word inst) {
	*d = (inst & d5bit) >> 4;
	*r = ((inst & 0x0600) >> 5) | (inst & r4bit);
}

void AVRInstructionSet::iwOperands(word * d, word * r, word inst) {
	*d = ((inst & 0x0030) >> 4) * 2 + 24; // 4 upper register pairs
	*r = ((inst & 0x00C0) >> 4) | (inst & r4bit);
}

void AVRInstructionSet::generateState() {
	retrieve();
	host->movePC();
}


int AVRInstructionSet::retrieve() {
	int wordCount = 1;
	std::stringstream msg;

	try {
		// retrieve
		inst = host->readWord(host->pc, Emulator::PROGRAM);

		if (host->getConfig(Emulator::VERBOSE))
			msg << "Read instruction 0x" << std::hex << host->pc << ": 0x" << inst;
			log(msg.str());

		//decode

		low = inst & 0xFF;
		high = inst >> 8;

		nib0 = low & 0xF;
		nib1 = (low & 0xF0) >> 4;
		nib2 = high & 0xF;
		nib3 = (high & 0xF0) >> 4;

		switch (nib3) {

			// ### Groups defined by nib3 ONLY

		case 0x00: // MOVW/MULS family
			if (inst == 0x0000) operation = NOP;
			break;

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

		case 0x08: // LDD/STD Z+k/Y+k
		case 0x0A:
			if ((nib2 & 0x2) == 0) { // LDD
				if ((nib0 & 0x8) == 0) operation = LDDZ;
				else operation = LDDY;
			}
			else { // STD
				if ((nib0 & 0x8) == 0) operation = STDZ;
				else operation = STDY;

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
						operation = LPM;
						break;
					}
					break;

				case 0x05:
					switch (selector) { // TODO
					case 0:
						operation = LPMp;
						break;
					case 2:
						operation = ASR;
						break;
					}
					break;
				case 0x06:
					switch (selector) { // TODO
					case 2:
						operation = LSR;
						break;
					}
					break;

				case 0x07:
					switch (selector) { // TODO
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
						case 0x0E:
							operation = SPM;
							break;
						}
					}
					break;

				case 0x09:
					switch (nib2) {
					case 4:
						if (nib1 == 0) {
							operation = IJMP;
							commit(2);
						}
						break;
					case 5:
						if (nib1 == 0) {
							operation = ICALL;
							commit(3);
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
			postCommit = true;
			break;
		}
	}

	catch (const char* except) {
		log(except);
	}

	return wordCount;
}

void AVRInstructionSet::applyState() {
	bool waiting = false;

	switch (operation) {

#pragma region Arithmetic and Logic

	case ADD:
		operands5bit(&d, &r, inst);

		opd = host->readByte(d);
		opr = host->readByte(r);
		resultByte = opd + opr;
		host->writeByte(d, resultByte);

		opstring = "ADD " + std::to_string(d) + ", " + std::to_string(r);
		break;

	case ADC:

			opstring = "ADC";

		break;

	case ADIW:
		iwOperands(&d, &r, inst);

		opd = host->readWord(d) + r;
		host->writeWord(d, opd);

		// Todo: SREG update

		break;

	case SUB:

		opstring = "SUB";

		break;

	case SUBI:

		opstring = "SUBI";

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

	case SBIW:
		iwOperands(&d, &r, inst);

		opd = host->readWord(d) - r;
		host->writeWord(d, opd);

		// Todo: SREG update

		break;

	case AND:
	case TST:
		// if r == d then TST


		opstring = "AND";

		break;

	case ANDI:
	case CBR:
		operands4bitAndConstant(&d, &k, inst);

		// Complement K for CBR.
		resultByte = host->readByte(d) & k;
		host->writeByte(d, resultByte);

		// Todo: SREG update, 

		opstring = "ANDI " + std::to_string(d) + ", $" + std::to_string(k);
		break;

	case OR: // 1 cycle
		operands5bit(&d, &r, inst);

		resultByte = host->readWord(d) | host->readWord(r);
		host->writeByte(d, resultByte);

		// todo: SREG update, S = N^V V = 0 N = R7, Z = (R == 0)
		opstring = "OR " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case ORI: // 1 cycle
	case SBR:
		operands4bitAndConstant(&d, &k, inst);

		resultByte = host->readByte(d) | k;
		host->writeByte(d, resultByte);

		// Todo: SREG update, S = N^V, V = 0, N = R7, Z = (R == 0)
		opstring = "ORI " + std::to_string(d) + ", $" + std::to_string(k);
		break;

	case EOR:
	case CLR:
		operands5bit(&d, &r, inst);
		
		resultByte = d ^ r;

		host->writeByte(d, resultByte);

		if (r == d) opstring = "CLR " + std::to_string(d);
		else opstring = "EOR " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case COM:

		opstring = "COM";

		break;

	case NEG: // 1 cycle
		operand5bit(&d, inst);
		resultByte = ~(host->readByte(d)) + 1;
		host->writeByte(d, resultByte);
		break;

	case INC:
		operand5bit(&d, inst);

		host->writeByte(d, host->readByte(d) + 1);

		// Todo: SREG update,
		// S: N^V
		// V: R7 AND !R6 AND !R5 AND !R4 AND !R3 AND !R2 AND !R1 AND !R0
		// N: R7
		// Z: = (R == 0)

		break;

	case DEC:
		operand5bit(&d, inst);

		host->writeByte(d, host->readByte(d) - 1);

		// Todo: SREG update,
		// S: N^V
		// V: !R7 AND R6 AND R5 AND R4 AND R3 AND R2 AND R1 AND R0
		// N: R7
		// Z: = (R == 0)

		break;

	case SER:

		opstring = "SER";

		break;

	case MUL:
		operands5bit(&d, &r, inst);

		resultWord = host->readByte(d) * host->readByte(r);
		host->writeWord(r0, resultWord);

		// Todo: Sreg updates. C = R15, z = (R == 0)
		opstring = "MUL " + std::to_string(d) + ", " + std::to_string(r);

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

		opstring = "FMUL";

		break;

	case FMULS:

		opstring = "FMULS";

		break;

	case FMULSU:

		opstring = "FMULSU";

		break;

#pragma endregion

#pragma region Branch instructions

	case RJMP: // 2 cycles
		operand24bit(&d, inst);

		host->setPC(d);

		opstring = "RJMP " + std::to_string(d);

		break;

	case IJMP:
		addr = host->readWord(rZ);
		host->setPC(addr);

		opstring = "IJMP > " + std::to_string(addr);

		break;

	case JMP:
		addr = ((inst & d5bit) << 13) | ((inst & 0x01) << 8) | host->readByte(host->pc);
		
		host->setPC(addr);

		opstring = "JMP > " + std::to_string(addr);
		break;

	case RCALL: // 3 cycles on 16bit PC, 4 cycles on 22bit PC. Assuming 16-bit.
		operand24bit(&d, inst);

		// Test: SP is 0x0F, push moves to 0x0D, 0x0E/0x0F are new contents.
		host->writeWord(host->readWord(SP) - 1, host->movePC()); // Store return address...
		host->writeWord(host->readWord(SP), host->readWord(SP) - 2); // Move SP to SP-2.

		// Call:
		host->setPC(d);

		opstring = "RCALL " + std::to_string(d);

		break;

	case ICALL:
		addr = host->readWord(rZ);

		host->writeWord(host->readWord(SP) - 1, host->movePC()); // Store return address in SP(-1:0)
		host->writeWord(host->readWord(SP), host->readWord(SP) - 2); // Move SP to SP-2.

		host->setPC(addr);

		opstring = "ICALL > " + std::to_string(addr);
		break;

	case CALL:

		host->writeWord(host->readWord(SP) - 1, host->pc);
		host->writeWord(SP, host->readWord(SP) - 2);

		addr = ((inst & d5bit) << 13) | ((inst & 0x01) << 8) | host->readByte(host->pc);

		host->setPC(addr);

		opstring = "CALL > " + std::to_string(addr);

		break;

	case RET: // 4 cycles on 16bit PC, 5 on 22bit PC
		host->setPC(host->readWord(SP) + 1);
		host->writeWord(host->readWord(SP), host->readWord(SP) + 2); // For 22bit this is 3 bytes rather than 2

		opstring = "RET";
		break;

	case RETI: // 4 cycles on 16bit PC, 5 on 22bit PC
		host->setPC(host->readWord(SP) + 1);
		host->writeWord(host->readWord(SP), host->readWord(SP) + 2); // For 22bit this is 3 bytes rather than 2

		host->setBit(SREG, interrupt);

		opstring = "RETI";

		break;

	case CPSE:
		operands5bit(&d, &r, inst);

		if (host->readByte(d) == host->readByte(r)) {
			host->movePC(retrieve());
			uncommit();
		}

		opstring = "CPSE " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case CP:

		opstring = "CP ";

		break;

	case CPC:

		opstring = "CPC ";

		break;

	case CPI:
		operands4bitAndConstant(&d, &r, inst);

		opstring = "CPI " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case SBRC:

		opstring = "SBRC ";

		break;

	case SBRS:

		opstring = "SBRS ";

		break;

	case SBIC:
		operands5bitAnd3bit(&d, &k, inst);

		if (!host->readBit(d, k)) {
			host->movePC(retrieve()); // check width to skip
			uncommit();
		}

		opstring = "SBIC " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case SBIS:
		operands5bitAnd3bit(&d, &k, inst);

		if (host->readBit(d, k)) {
			host->movePC(retrieve()); // check width to skip
			uncommit();
		}

		opstring = "SBIS " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case BRBS:
		operands6bitAnd3bit(&d, &r, inst);

		// allow for postcommitting:
		if (postCommit) {
			postCommit = false;
			if (host->readBit(SREG, r)) {
				commit(0);
				waiting = true;
				break;
			}
		}

		waiting = false;

		if (host->readBit(SREG, r)) {
			host->movePC((int16_t)d);
		}

		opstring = std::to_string(d);

		switch (r) {
		case 0x00:
			opstring = "BRCS " + opstring;
			break;
		case 0x01:
			opstring = "BREQ " + opstring;
			break;
		case 0x02:
			opstring = "BRMI " + opstring;
			break;
		case 0x03:
			opstring = "BRVS " + opstring;
			break;
		case 0x04:
			opstring = "BRLT " + opstring;
			break;
		case 0x05:
			opstring = "BRHS " + opstring;
			break;
		case 0x06:
			opstring = "BRTS " + opstring;
			break;
		case 0x07:
			opstring = "BRIE " + opstring;
			break;
		default:
			opstring = "BRBS " + opstring + ", " + std::to_string(r);
			break;
		}

		break;

	case BRBC:
		operands6bitAnd3bit(&d, &r, inst);

		// allow for postcommitting:
		if (postCommit) {
			postCommit = false;
			if (!host->readBit(SREG, r)) {
				commit(0);
				waiting = true;
				break;
			}
		}

		waiting = false;

		if (!host->readBit(SREG, r)) {
			host->movePC((int16_t)d);
		}

		opstring = std::to_string(d);

		switch (r) {
		case 0x00:
			opstring = "BRCC " + opstring;
			break;
		case 0x01:
			opstring = "BRNE " + opstring;
			break;
		case 0x02:
			opstring = "BRPL " + opstring;
			break;
		case 0x03:
			opstring = "BRVC " + opstring;
			break;
		case 0x04:
			opstring = "BRGE " + opstring;
			break;
		case 0x05:
			opstring = "BRHC " + opstring;
			break;
		case 0x06:
			opstring = "BRTT " + opstring;
			break;
		case 0x07:
			opstring = "BRID " + opstring;
			break;
		default:
			opstring = "BRBC " + opstring + ", " + std::to_string(r);
			break;
		}

		break;

#pragma endregion

#pragma region Bit and Bit-test

	case SBI: // 2 cycles
		operands5bitAnd3bit(&d, &k, inst);

		host->setBit(d, k);

		opstring = "SBI " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case CBI:
		operands5bitAnd3bit(&d, &k, inst);

		host->clearBit(d, k);

		opstring = "CBI " + std::to_string(d) + ", " + std::to_string(k);

		break;

	case LSL:

		opstring = "LSL ";

		break;

	case LSR:

		opstring = "LSR ";

		break;

	case ROL:

		opstring = "ROL ";

		break;

	case ROR: // 1 cycle
		operand5bit(&d, inst);

		// Todo: Implement. R >> 1, C to b7, b0 to C

		// Todo: SREG update S = N^V, V = N^C (after), N = R7, Z = (R == 0), C = Rd0

		opstring = "ROR " + std::to_string(d);

		break;

	case ASR:

		opstring = "ASR ";

		break;

	case SWAP:
		operand5bit(&d, inst);

		opd = host->readByte(d);
		host->writeByte(d, ((opd & 0xFF00) >> 4) | ((opd & 0x00FF) << 4));

		opstring = "SWAP " + std::to_string(d);
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

	case BST:

		opstring = "BST ";

		break;

	case BLD:

		opstring = "BLD ";

		break;

#pragma endregion

#pragma region Data transfer

	case MOV: // 1 cycle
		operands5bit(&d, &r, inst);
		host->writeByte(d, host->readByte(r));

		opstring = "MOV " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case MOVW: // 1 cycle
		operands4bit(&d, &r, inst);

		host->writeWord(d, host->readWord(r));

		opstring = "MOVW " + std::to_string(d) + ", " + std::to_string(r);
		break;

	case LDI:

		opstring = "LDI ";

		break;

	case LDX:

		opstring = "LDX ";

		break;

	case LDXp:

		opstring = "LDX+ ";

		break;

	case LDmX:

		opstring = "LD-X ";

		break;

	case LDY:
	case LDDY:
		addr = host->readWord(rY) + k;

		opstring = "LDY ";

		if (k != 0) opstring += "+k";
		break;

	case LDYp:

		opstring = "LDY+";

		break;

	case LDmY:

		opstring = "LD-Y";

		break;
		
	case LDZ:
	case LDDZ:

		addr = host->readWord(rZ) + k;
		host->writeByte(d, host->readByte(addr));

		opstring = "LDZ " + std::to_string(d);
		if (k != 0) opstring += " +k " + std::to_string(k);

		break;
	
	case LDZp:

		opstring = "LDZ+";

		break;

	case LDmZ:

		opstring = "LD-Z";

		break;

	case LDS:

		opstring = "LDS";

		break;

	case STX:

		opstring = "STX";

		break;

	case STXp:

		opstring = "STX+";

		break;

	case STmX:

		opstring = "ST-X";

		break;

	case STY:
	case STDY:
		d = host->readWord(rY) + k;


		opstring = "STY ";


		if (k != 0) opstring += "+k";
		break;

	case STYp:

		opstring = "STY+";


		break;

	case STmY:

		opstring = "ST-Y";

		break;

	case STZ:
	case STDZ:
		addr = host->readWord(rZ) + k;
		host->writeByte(addr, host->readByte(d));

		opstring = "STDZ " + std::to_string(d);
		if (k != 0) opstring += " +k " + std::to_string(k);

		break;

	case STZp:

		opstring = "STZ+ ";

		break;

	case STmZ:

		opstring = "ST-Z ";

		break;

	case STS:

		opstring = "STS ";

		break;

	case LPM:

		opstring = "LPM";

		break;

	case LPMp:

		opstring = "LPMZ+";

		break;

	case SPM:

		opstring = "SPM";

		break;

	case IN:
		ioOperands(&d, &r, inst);

		r += MMIO_OFFSET; // moves from IO space (0x00-0x3F) -> data space (0x20-0x5F) and thus prevents mirroring

		host->writeByte(d, host->readByte(r));

		opstring = "IN " + std::to_string(d) + ", " + std::to_string(r);

		break;

	case OUT: // 1 cycle
		ioOperands(&d, &r, inst);

		r += MMIO_OFFSET; // moves from IO space (0x00-0x3F) -> data space (0x20-0x5F) and thus prevents mirroring

		host->writeByte(r, host->readByte(d));

		opstring = "OUT " + std::to_string(d) + ", " + std::to_string(r);

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
#pragma endregion

#pragma region MCU Control

	case NOP: // 1 cycle

		opstring = "NOP";

		break;

	case SLEEP:

		opstring = "SLEEP";

		break;

	case WDR:

		opstring = "WDR";

		break;

	case BREAK:
		host->setState(Emulator::STOP);
		opstring = "BREAK";
		break;

#pragma endregion

	default:
		break;
	}

	if (waiting) return;

	host->writeByte(SREG, sreg); // Replace SREG with new sreg

	if (host->getConfig(Emulator::VERBOSE)) {
		log("-> " + opstring + " " + std::to_string(inst));
	}

	// reset sentinel values for transient stuff
	sreg = 0xFF;
	operation = UNKN;
	opstring = "Unknown opcode";
}
std::string AVRInstructionSet::name() {
	return "AVR Instruction set";
}