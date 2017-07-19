
#include <iostream>
#include "avrinstructionset.h"

void AVRInstructionSet::buildNewSreg8() {
	// C: (Rd7 AND Rr7 OR Rr7 AND !R7 or !R7 AND !R0)
	if (
		((opd & (1 << 7)) && (opr & (1 << 7))) ||
		((opr & (1 << 7)) && !(result8 & (1 << 7))) ||
		(!(result8 & (1 << 7)) && !(result8 & (1 << 0)))
	) sreg |= (1 << carry);
	else sreg &= ~(1 << carry);

	// Z: (!R7:1)
	if (result8 & 0xFF) sreg |= (1 << zero);
	else sreg &= ~(1 << zero);

	// N: (R7)
	if (result8 & 0x80) sreg |= (1 << neg);
	else sreg &= ~(1 << neg);

	// V: (Rd7 AND Rr7 AND !R7 OR !Rd7 AND Rr7 AND R7)
	if (
		(opd & (1 << 7)) && (opr & (1 << 7)) && (~result8 & (1 << 7)) ||
		(~opd & (1 << 7)) && (~opr & (1 << 7)) && (result8 & (1 << 7))
	) sreg |= (1 << overflow);
	else sreg &= ~(1 << overflow);
	
	// S: sign is always XOR neg/overflow.
	sreg &= ~(1 << sign); // clear bit
	sreg |= (((sreg >> neg) ^ (sreg >> overflow)) & 0x01) << sign; // set result
	
	// H: (Rd3 AND Rr3 OR Rr3 AND !R3 OR !R3 AND Rd3)
	if (
		((opd & (1 << 3)) && (opr & (1 << 3))) ||
		((opr & (1 << 3)) && (~result8 & (1 << 3))) ||
		((~result8 & (1 << 3)) && (opd & (1 << 3)))
	) sreg |= (1 << halfcarry);
	else sreg &= ~(1 << halfcarry);
}

void AVRInstructionSet::buildNewSreg16() {
	// C: (Rd7 AND Rr7 OR Rr7 AND !R7 or !R7 AND !R0)
	if (
		((opd & (1 << 7)) && (opr & (1 << 7))) ||
		((opr & (1 << 7)) && !(result8 & (1 << 7))) ||
		(!(result8 & (1 << 7)) && !(result8 & (1 << 0)))
		) sreg |= (1 << carry);
	else sreg &= ~(1 << carry);

	// Z: (!R7:1)
	if (result8 & 0xFF) sreg |= (1 << zero);
	else sreg &= ~(1 << zero);

	// N: (R7)
	if (result8 & 0x80) sreg |= (1 << neg);
	else sreg &= ~(1 << neg);

	// V: (Rd7 AND Rr7 AND !R7 OR !Rd7 AND Rr7 AND R7)
	if (
		(opd & (1 << 7)) && (opr & (1 << 7)) && (~result8 & (1 << 7)) ||
		(~opd & (1 << 7)) && (~opr & (1 << 7)) && (result8 & (1 << 7))
		) sreg |= (1 << overflow);
	else sreg &= ~(1 << overflow);

	// S: sign is always XOR neg/overflow.
	sreg &= ~(1 << sign); // clear bit
	sreg |= (((sreg >> neg) ^ (sreg >> overflow)) & 0x01) << sign; // set result

	// H: (Rd3 AND Rr3 OR Rr3 AND !R3 OR !R3 AND Rd3)
	if (
		((opd & (1 << 3)) && (opr & (1 << 3))) ||
		((opr & (1 << 3)) && (~result8 & (1 << 3))) ||
		((~result8 & (1 << 3)) && (opd & (1 << 3)))
		) sreg |= (1 << halfcarry);
	else sreg &= ~(1 << halfcarry);

}

void AVRInstructionSet::generateState() {
		try {

			// retrieve
			if (host->getConfig(Emulator::VERBOSE)) log("Reading instruction 0x" + std::to_string(host->pc));
			word inst = host->readWord(host->pc);
			host->pc += host->instructionSize;

			//decode & act

			byte low = inst & 0xFF;
			byte high = inst >> 8;

			byte nib0 = low & 0xF;
			byte nib1 = (low & 0xF0) >> 4;
			byte nib2 = high & 0xF;
			byte nib3 = (high & 0xF0) >> 4;

			if (inst == NOP) { // 1 cycle
				commit(3);
				opstring = "NOP";
			}

			else if (inst == HLT) { // 0 cycle
				opstring = "HLT";
				host->setState(Emulator::STOP);
			}
			else {
				switch (nib3) {

					// ### Groups defined by nib3 ONLY

				case 0x03: // CPI only
					k = (nib2 << 8) | nib0;
					d = nib1;

					reg = host->readByte(d);

					opstring = "CPI";
					break;

				case 0x04: // SBCI only
					opstring = "SBCI";
					break;

				case 0x05: // SUBI only
					opstring = "SUBI";
					break;

				case 0x06: // ORI/SBR only
					opstring = "ORI/SBR";
					break;

				case 0x07: // ANDI/CBR only
					opstring = "ANDI/CBR";
					break;

					// Groups defined by nib3 AND nib2

				case 0x00: // MOVW/MULS family
					d = (inst & d5bit) >> 4;
					r = ((inst & r5bith) >> 5) | (inst & nib0);

					switch (nib2) {
					case 0x01:
						opstring = "MOVW";
						break;
					case 0x02:
						opstring = "MULS";
						break;
					case 0x03:
						if (nib0 <= 7 && nib1 <= 7) {
							opstring = "MULSU";
						}
						else if (nib0 <= 7 && nib1 > 7) {
							opstring = "FMUL";
						}
						else if (nib0 > 7 && nib1 <= 7) {
							opstring = "FMULS";
						}
						else {
							opstring = "FMULSU";
						}
						break;
					case 0x08:
						opstring = "MULS";
						break;
					default:
						if (nib2 <= 0x07) { // TODO
							opstring = "CPC";
						}
						else if (nib2 <= 0x0B) { // TODO
							opstring = "SBC";
						}
						else { // TODO
							opd = host->readByte(d);
							opr = host->readByte(r);
							result8 = opr + opd;

							opstring = "ADD";
						}
					}
					break;

				case 0x01: // CPSE/CP/SUB family

					if (nib2 <= 0x03) { // CPSE
						d = (inst & d5bit) >> 4;
						r = ((inst & r5bith) >> 5) | (inst & nib0);

						if (host->readByte(d) == host->readByte(r)) {
							host->pc += host->instructionSize;
							// TODO: Add handling for double-word instructions
						}

						opstring = "CPSE";
					}
					else if (nib2 <= 0x07) { // TODO
						opstring = "CP";
					}
					else if (nib2 == 0x0B) { // TODO
						opstring = "SUB";
					}
					else { // TODO
						int result = 0x00;
						opstring = "ADC";
					}
					break;

				case 0x02: // AND/EOR/OR/MOV family
					d = (inst & d5bit) >> 4;
					r = ((inst & r5bith) >> 5) | (inst & nib0);

					if (nib2 <= 0x03) { // TODO
						if (d == r) {
							opstring = "TST";
						}
						else {
							opstring = "AND";
						}
					}
					else if (nib2 <= 0x07) { // TODO
						if (d == r) {
							opstring = "CLR";
						}
						else {
							opstring = "EOR";
						}
					}
					else if (nib2 <= 0x0B) { // TODO
						opstring = "OR";
					}
					else { // TODO
						result8 = host->readByte(r);

						opstring = "MOV";
					}
					break;


				case 0x08: // LDD/STD Z+k/Y+k
				case 0x0A:
					k = ((nib3 & 0x2) << 2) | (nib0 & 0x07);

					if ((nib2 & 0x2) == 0) { // LDD
						d = ((nib2 & 0x01) << 4) | nib1;
						opstring = "LDD ";

						if ((nib0 & 0x8) == 0) { // Z+k
							addr = host->readWord(rZ) + k;

							opstring += "Z";
						}
						else { // Y+k
							addr = host->readWord(rY) + k;
							opstring += "Y";
						}

						result8 = host->readByte(addr);
					}
					else { // STD
						r = ((nib2 & 0x01) << 4) | nib1;
						opstring = "STD ";
						if ((nib0 & 0x8) == 0) { // Z+k
							d = host->readWord(rZ) + k;
							opstring += "Z";
						}
						else { // Y+k
							d = host->readWord(rY) + k;
							opstring += "Y";
						}
						result8 = host->readByte(r);
					}
					if (k != 0) opstring += "+k";
					break;

				case 0x09:

					// patterns of 0-1, 2-3, 4-5 can be condensed to 0, 1, 2 by nib2/2

					selector = nib2 / 2;

					switch (nib0) {
					case 0x00:
						switch (selector) { // TODO
						case 0:
							opstring = "LDSI";
							break;
						case 1:
							opstring = "STSI";
							break;
						case 2:
							opstring = "COM";
							break;
						}
						break;

					case 0x01:
						switch (selector) { // TODO
						case 0:
							opstring = "LDZ+";
							break;
						case 1:
							opstring = "STZ+";
							break;
						case 2:
							opstring = "NEG";
							break;
						}
						break;

					case 0x02:
						switch (selector) { // TODO
						case 0:
							opstring = "LD-Z";
							break;
						case 1:
							opstring = "ST-Z";
							break;
						case 2:
							opstring = "SWAP";
							break;
						}
						break;

					case 0x03:
						opstring = "INC";
						break;
					case 0x04:
						switch (selector) { // TODO
						case 0:
							opstring = "LMP Z";
							break;
						case 1:
							opstring = "XCH";
							break;
						}
						break;

					case 0x05:
						switch (selector) { // TODO
						case 0:
							opstring = "LPM Z+";
							break;
						case 1:
							opstring = "LAS";
							break;
						case 2:
							opstring = "ASR";
							break;
						}
						break;
					case 0x06:
						switch (selector) { // TODO
						case 0:
							opstring = "ELMP Z";
							break;
						case 1:
							opstring = "LAC";
							break;
						case 2:
							opstring = "LSR";
							break;
						}
						break;

					case 0x07:
						switch (selector) { // TODO
						case 0:
							opstring = "ELPM z+";
							break;
						case 1:
							opstring = "LAT";
							break;
						case 2:
							opstring = "ROR";
							break;
						}
						break;

					case 0x08:
						if (nib2 == 0x04) {
							d = (nib1 & 0x7) >> 1;

							if (nib1 <= 0x07) {
								host->setBit(SREG, d);
								commit(3);
								opstring = "BSET ";
							}
							else {
								host->clearBit(SREG, d);
								opstring = "BCLR ";
							}

							switch (nib1) {
							case 0x00:
								opstring += "SEC";
								break;
							case 0x01:
								opstring += "SEZ";
								break;
							case 0x02:
								opstring += "SEN";
								break;
							case 0x03:
								opstring += "SEV";
								break;
							case 0x04:
								opstring += "SES";
								break;
							case 0x05:
								opstring += "SEH";
								break;
							case 0x06:
								opstring += "SET";
								break;
							case 0x07:
								opstring += "SEI";
								break;
							case 0x08:
								opstring += "CLC";
								break;
							case 0x09:
								opstring += "CLZ";
								break;
							case 0x0A:
								opstring += "CLN";
								break;
							case 0x0B:
								opstring += "CLV";
								break;
							case 0x0C:
								opstring += "CLS";
								break;
							case 0x0D:
								opstring += "CLH";
								break;
							case 0x0E:
								opstring += "CLT";
								break;
							case 0x0F:
								opstring += "CLI";
								break;
							}
						}
						else if (nib2 == 0x05) {
							switch (nib1) {
							case 0x00:
								opstring = "RET";
								break;
							case 0x01:
								opstring = "RETI";
								break;
							case 0x02:
								opstring = "RET";
								break;
							case 0x08:
								opstring = "SLEEP";
								break;
							case 0x09:
								opstring = "BREAK";
								break;
							case 0x0A:
								opstring = "WDR";
								break;
							case 0x0C:
								opstring = "LPM";
								break;
							case 0x0D:
								opstring = "ELPM";
								break;
							case 0x0E:
								opstring = "SPM";
								break;
							case 0x0F:
								opstring = "SPM Z+";
								break;
							}
						}
						break;

					case 0x09:
						addr = host->readWord(rZ);
						switch (nib2) {
						case 4:
							if (nib1 == 0) {
								
								host->pc = addr;
								opstring = "IJMP Z";
							}
							else {
								opstring = "EIJMP Z";
							}
							break;
						case 5:
							if (nib1 == 0) { // TODO
								host->pc = addr;
								opstring = "ICALL Z";
							}
							else {
								opstring = "EICALL Z";
							}
							break;
						}

						//opstring = "LDY+/STY+";
						break;

					case 0x0A:
						opstring = "LD-Y/ST-Y";
						break;

					case 0x0B:
						break;

					case 0x0C:
						opstring = "LD X/ST X // JMPABS22";
						break;

					case 0x0D:
						opstring = "LD X+/ST X+ // JMPABS22";
						break;

					case 0x0E:
						opstring = "LD -X/ST -X // CALLABS22";
						break;

					case 0x0F:
						opstring = "PUSH/POP // CALLABS22";
						break;
					}
					break;

				case 0x0B:
					if (nib2 <= 0x07) opstring = "IN";
					else opstring = "OUT";
					break;

				case 0x0C:
					opstring = "RJMP";
					break;

				case 0x0D:
					opstring = "RCALL";
					break;

				case 0x0E:
					opstring = "LDI";
					if (inst & 0x0F0F) opstring = "SER";
					break;

				case 0x0F:
					if (nib2 <= 3) opstring = "BRBS";
					else opstring = "BRBC";
					break;
				}
			}

		}
		catch (const char* except) {
			log(except);
		}
}

void AVRInstructionSet::applyState() {
	// TODO: Handle initial cycle having no state to apply.

	host->writeByte(SREG, sreg); // Replace SREG with new sreg

	if (host->getConfig(Emulator::VERBOSE)) {
		log("Applying new state for " + opstring + " r: 0x" + std::to_string(r) +
			" d: 0x" + std::to_string(d) + " k: 0x" + std::to_string(k));
	}

	host->writeByte(d, result8);

	// reset sentinel values for transient stuff
	sreg = 0xFF;
	opstring = "Unknown opcode"; // overwritten by recognised ops
	// End TODO.
}

