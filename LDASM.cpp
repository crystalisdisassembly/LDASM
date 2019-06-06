//Portions of code adapted from dcc6502 source at https://github.com/tcarmelveilleux/dcc6502/blob/master/dcc6502.c

#define _CRT_SECURE_NO_WARNINGS 1

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "6502 Instruction Set.h"
//#include "Instruction.h"

unsigned char g_ucIsLabel[0x4000];

bool IsCDLVerifiedCode(unsigned char ucCDLByte)
	{
	return ucCDLByte & 0x1; //0x1 = bit flag for verified code
	}

bool IsCDLVerifiedData(unsigned char ucCDLByte)
	{
	return ucCDLByte & 0x2; //0x2 = bit flag for verified data
	}

bool IsCDLVerifiedEntryPoint(unsigned char ucCDLByte)
	{
	return ucCDLByte & 0x80; //0x80 = bit flag for verified entry point
	}

unsigned short DisassembleSingleInstruction(char *pszResultBuffer, unsigned char *pucPRGBuffer, unsigned char *pucCDLBuffer, unsigned short unOffsetStart, unsigned short unLengthPRGBuffer, unsigned short unMappedBaseAddress, int nPRGBank, int nCHRBank)
	{
	unsigned char	ucOpcode = 0; //the opcode read from the PRG buffer
	int				ucOpcodeIndex = 0; //index of the opcode in the opcode array
	bool			bFound = false; //if the opcode was found in the opcode array

	unsigned char	ucByteOperand = 0; //byte operand from the PRG buffer
	unsigned short	unWordOperand = 0; //word operand from the PRG buffer

	unsigned short	unBytesRead = 0; //number of bytes read (instruction + any operand) to disassemble the instruction

	unsigned short	unDisplacement = 0; //for relative mode addressing calculation

	if (g_ucIsLabel[unOffsetStart])
		{
		sprintf(pszResultBuffer, "\nL_%s_0x%01X_0x%04X:\n", ((nPRGBank != -1) ? "PRG" : "CHR"), ((nPRGBank != -1) ? nPRGBank : nCHRBank), unOffsetStart);
		g_ucIsLabel[unOffsetStart] = false;

		return 0;
		}

	if (unOffsetStart >= unLengthPRGBuffer)
		{
		sprintf(pszResultBuffer, "Error: Passed EOF");
		return 0;
		}

	ucOpcode = pucPRGBuffer[unOffsetStart + unBytesRead++];

	//need more robust code here to process .db stuff.. like if it's not verified code, marked as an entry point (see commented out parts below)
	if (IsCDLVerifiedData(pucCDLBuffer[unOffsetStart]) && (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart])))
		{
		int nLineSize = 0;
		bool bLabel = false;

		unBytesRead--; //reset the reading from to the current opcode, since we may be making more reads but don't want to push the counter too far in case the next byte isn't data

		sprintf(pszResultBuffer, "\n;---- Start CDL Confirmed Data Block: Offset 0x%04X ----", unOffsetStart + unBytesRead);

		//print out up to 128 bytes in a single .db block
		while ((IsCDLVerifiedData(pucCDLBuffer[unOffsetStart + unBytesRead])) &&
			   (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart + unBytesRead])) &&
			   (unBytesRead < 0x80) &&
			   ((unOffsetStart + unBytesRead) < ((nPRGBank != -1) ? (0x4000) : (0x2000))))
			{
			if (g_ucIsLabel[unOffsetStart + unBytesRead])
				nLineSize = 0;

			if ((nLineSize % 8) == 0)
				{
				if (g_ucIsLabel[unOffsetStart + unBytesRead])
					{
					g_ucIsLabel[unOffsetStart + unBytesRead] = false;
					sprintf((pszResultBuffer + strlen(pszResultBuffer)) - 2 /*get rid of the terminal ", "*/, "\n\nL_%s_0x%01X_0x%04X:\n\n.byte", ((nPRGBank != -1) ? "PRG" : "CHR"), ((nPRGBank != -1) ? nPRGBank : nCHRBank), (unOffsetStart + unBytesRead));
					}
				else
					sprintf((pszResultBuffer + strlen(pszResultBuffer)) - 2 /*get rid of the terminal ", "*/, "\n.byte");
				}

			sprintf((pszResultBuffer + strlen(pszResultBuffer)), " $%02X, ", ucOpcode);

			ucOpcode = pucPRGBuffer[unOffsetStart + (++unBytesRead)];
			nLineSize++;
			}
		sprintf(pszResultBuffer + strlen(pszResultBuffer) - 2 /*get rid of the terminal ", "*/, "\n;---- End CDL Confirmed Data Block: Total Bytes 0x%02X ----\n", (unBytesRead));

		return unBytesRead;
		}
	else if (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart]))
		{
		int nLineSize = 0;
		
		unBytesRead--; //reset the reading from to the current opcode, since we may be making more reads but don't want to push the counter too far in case the next byte isn't data

		sprintf(pszResultBuffer, "\n;---- Start CDL Unknown Block: Offset 0x%04X ----", unOffsetStart + unBytesRead);

		//print out up to 128 bytes in a single .db block
		while ((!IsCDLVerifiedData(pucCDLBuffer[unOffsetStart + unBytesRead])) &&
			   (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart + unBytesRead])) &&
			   (unBytesRead < 0x80) &&
			   ((unOffsetStart + unBytesRead) < ((nPRGBank != -1) ? (0x4000) : (0x2000))))
			{
			if (g_ucIsLabel[unOffsetStart + unBytesRead])
				nLineSize = 0;

			if ((nLineSize % 8) == 0)
				{
				if (g_ucIsLabel[unOffsetStart + unBytesRead])
					{
					g_ucIsLabel[unOffsetStart + unBytesRead] = false;
					sprintf((pszResultBuffer + strlen(pszResultBuffer)) - 2 /*get rid of the terminal ", "*/, "\n\nL_%s_0x%01X_0x%04X:\n\n.byte", ((nPRGBank != -1) ? "PRG" : "CHR"), ((nPRGBank != -1) ? nPRGBank : nCHRBank), (unOffsetStart + unBytesRead));
					}
				else
					sprintf((pszResultBuffer + strlen(pszResultBuffer)) - 2 /*get rid of the terminal ", "*/, "\n.byte");
				}

			sprintf((pszResultBuffer + strlen(pszResultBuffer)), " $%02X, ", ucOpcode);

			ucOpcode = pucPRGBuffer[unOffsetStart + (++unBytesRead)];
			nLineSize++;
			}
		sprintf(pszResultBuffer + strlen(pszResultBuffer) - 2 /*get rid of the terminal ", "*/, "\n;---- End CDL Unknown Block: Total Bytes 0x%02X ----\n", (unBytesRead));

		return unBytesRead;
		}
	/*
	//Verify that the matching byte in the CDL is code -- if not, then process as data or error and return
	if (IsCDLVerifiedData(pucCDLBuffer[unOffsetStart]) && (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart])))
		{
		sprintf(pszResultBuffer, ".db $%02X\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X .. Verified data in CDL", ucOpcode, unOffsetStart, ucOpcode);

		if (IsCDLVerifiedEntryPoint(pucCDLBuffer[unOffsetStart]))
			sprintf(pszResultBuffer, ".db $%02X\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X .. ERROR::: Above byte was flagged as entry point in CDL but was verified as data, not code", ucOpcode, unOffsetStart, ucOpcode);

		return unBytesRead;
		}
	else if (!IsCDLVerifiedCode(pucCDLBuffer[unOffsetStart]))
		{
		sprintf(pszResultBuffer, ".db $%02X\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X .. Unknown if code or data in CDL", ucOpcode, unOffsetStart, ucOpcode);

		if (IsCDLVerifiedEntryPoint(pucCDLBuffer[unOffsetStart]))
			sprintf(pszResultBuffer, ".db $%02X\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X .. ERROR::: Above byte was flagged as entry point in CDL but was not verified as code or data", ucOpcode, unOffsetStart, ucOpcode);

		return unBytesRead;
		}*/

	//look to see if the opcode matches the instruction set
	for (int i = 0; ((i < NUM_OPCODES) && (!bFound)); i++)
		{
		if (ucOpcode == _Opcodes[i].ucByte)
			{
			ucOpcodeIndex = i;
			bFound = true;
			}
		}

	//if the opcode isn't found in the instruction set then output as .db and flag it as an illegal opcode
	//this is also useful to catch bugs in disassembly since they can throw off alignment on future reads from the PRG buffer which may be caught as illegal opcodes
	if (!bFound)
		{
		sprintf(pszResultBuffer, ".byte $%02X\t\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X .. Illegal Opcode!!", ucOpcode, unOffsetStart, ucOpcode);
		return unBytesRead;
		}

	//checks to prevent reading beyond EOF/end of bank
	switch (_Opcodes[ucOpcodeIndex]._enAddressingMode)
		{
		case IMMEDIATE: //1 byte operand
		case ZERO_PAGE:
		case ZERO_PAGE_INDEXED_X:
		case ZERO_PAGE_INDEXED_Y:
		case INDEXED_INDIRECT_X:
		case INDIRECT_INDEXED_Y:
		case RELATIVE:
			if ((unOffsetStart + unBytesRead) > ((nPRGBank == -1) ? 0x1FFF : 0x3FFF)) //if there's not enough space left to read an operand
				{
				sprintf(pszResultBuffer, ".byte $%02X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X - ERROR: %s instruction as code, but end of buffer reached before operand", _Opcodes[ucOpcodeIndex].ucByte, unOffsetStart, ucOpcode, _Opcodes[ucOpcodeIndex].szName);
				return unBytesRead;
				}
			break;
		case ABSOLUTE: //2 byte operand
		case INDIRECT_ABSOLUTE:
		case ABSOLUTE_INDEXED_X:
		case ABSOLUTE_INDEXED_Y:
			if ((unOffsetStart + unBytesRead) > ((nPRGBank == -1) ? 0x1FFF : 0x3FFF)) //if there's not enough space left to read an operand
				{
				sprintf(pszResultBuffer, ".byte $%02X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X - ERROR: %s instruction as code, but end of buffer reached before operand", _Opcodes[ucOpcodeIndex].ucByte, unOffsetStart, ucOpcode, _Opcodes[ucOpcodeIndex].szName);
				return unBytesRead;
				}
			else if ((unOffsetStart + unBytesRead) > ((nPRGBank == -1) ? 0x1FFE : 0x3FFE)) //if there's not enough space left to read the full 2 bytes of an operand
				{
				sprintf(pszResultBuffer, ".byte $%02X, $%02X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X - ERROR: %s instruction as code, but end of buffer reached before full operand", _Opcodes[ucOpcodeIndex].ucByte, pucPRGBuffer[unOffsetStart + unBytesRead + 1], unOffsetStart, ucOpcode, pucPRGBuffer[unOffsetStart + unBytesRead + 1], _Opcodes[ucOpcodeIndex].szName);
				unBytesRead++;
				return unBytesRead;
				}
			break;
		default: //no operand, so don't have to do anything here
			break;
		}
	//EOF/end of bank checks passed

	//Different addressing modes have different operand types -- process that here and then print the complete disassembly for the current instruction
	switch (_Opcodes[ucOpcodeIndex]._enAddressingMode)
		{
		case IMMEDIATE:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s #$%02X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case ABSOLUTE:
			unWordOperand = *((unsigned short *)(pucPRGBuffer + unOffsetStart + unBytesRead));
			unBytesRead += 2;

			if (unWordOperand < 0x100)
				sprintf(pszResultBuffer, "%s a:$%04X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			else
				sprintf(pszResultBuffer, "%s $%04X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			break;
		case ZERO_PAGE:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s $%02X\t\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X ", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case IMPLIED:

			sprintf(pszResultBuffer, "%s\t\t\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X ", _Opcodes[ucOpcodeIndex].szName, unOffsetStart, ucOpcode);
			break;
		case INDIRECT_ABSOLUTE:
			unWordOperand = *((unsigned short *)(pucPRGBuffer + unOffsetStart + unBytesRead));
			unBytesRead += 2;

			sprintf(pszResultBuffer, "%s ($%04X)\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			break;
		case ABSOLUTE_INDEXED_X:
			unWordOperand = *((unsigned short *)(pucPRGBuffer + unOffsetStart + unBytesRead));
			unBytesRead += 2;

			if (unWordOperand < 0x100)
				sprintf(pszResultBuffer, "%s a:$%04X, X\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			else
				sprintf(pszResultBuffer, "%s $%04X, X\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			break;
		case ABSOLUTE_INDEXED_Y:
			unWordOperand = *((unsigned short *)(pucPRGBuffer + unOffsetStart + unBytesRead));
			unBytesRead += 2;

			if (unWordOperand < 0x100)
				sprintf(pszResultBuffer, "%s a:$%04X, Y\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			else
				sprintf(pszResultBuffer, "%s $%04X, Y\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, unWordOperand, unOffsetStart, ucOpcode, unWordOperand & 0x00FF, (unWordOperand & 0xFF00) >> 8);
			break;
		case ZERO_PAGE_INDEXED_X:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s $%02X, X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case ZERO_PAGE_INDEXED_Y:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s $%02X, Y\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case INDEXED_INDIRECT_X:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s ($%02X, X)\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case INDIRECT_INDEXED_Y:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			sprintf(pszResultBuffer, "%s ($%02X), Y\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand);
			break;
		case RELATIVE:
			ucByteOperand = pucPRGBuffer[unOffsetStart + unBytesRead];
			unBytesRead++;

			//compute displacement from first byte after full instruction
			unDisplacement = unOffsetStart + 2;
			if (ucByteOperand > 0x7Fu)
				unDisplacement -= ((~ucByteOperand & 0x7FU) + 1);
			else
				unDisplacement += ucByteOperand & 0x7Fu;

			unDisplacement = unDisplacement + unMappedBaseAddress;

			//only print label if we're in a PRG bank, otherwise print the raw operand
			//in fact, emit the raw byte code with a commented instruction (ca65 will try and calculate displacement for relative operands so invalid addresses will cause errors)
			if (nCHRBank == -1)
				sprintf(pszResultBuffer, "%s L_%s_0x%01X_0x%04X\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X (computed address for relative mode instruction 0x%04X)", _Opcodes[ucOpcodeIndex].szName, ((nPRGBank != -1) ? "PRG" : "CHR"), ((nPRGBank != -1) ? nPRGBank : nCHRBank), (unDisplacement & 0xFFFF)/*ucByteOperand*/, unOffsetStart, ucOpcode, ucByteOperand, (unDisplacement & 0xFFFF));
			else
				{
				sprintf(pszResultBuffer,".byte $%02X, $%02X\t\t\t\t\t\t;%s $%02X\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X (computed address for relative mode instruction 0x%04X)", _Opcodes[ucOpcodeIndex].ucByte, ucByteOperand, _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand, (unDisplacement & 0xFFFF));
				//sprintf(pszResultBuffer, "%s $%02X\t\t\t;Offset: 0x%X, Byte Code: 0x%02X 0x%02X (computed address for relative mode instruction 0x%04X)", _Opcodes[ucOpcodeIndex].szName, ucByteOperand, unOffsetStart, ucOpcode, ucByteOperand, (unDisplacement & 0xFFFF));
				}
			break;
		case ACCUMULATOR:
			sprintf(pszResultBuffer, "%s A\t\t\t\t\t\t\t;Offset: 0x%X, Byte Code: 0x%02X", _Opcodes[ucOpcodeIndex].szName, unOffsetStart, ucOpcode);
			break;
		default:
			break;
		}

	return unBytesRead;
	}

//unMappedBaseAddress should be 0x8000, 0xC000, etc.
/*
unsigned short DisassembleBank(char *pszFileNameBank, char *pszFileNameCDL, char *pszFileNameASM, bool bPRGBank, unsigned short unMaxLength, unsigned short unMappedBaseAddress)
	{
	unsigned char *pucBankBuffer = (pszFileNameBank) ? (unsigned char *)malloc(unMaxLength * sizeof(unsigned char)) : NULL;
	unsigned char *pucCDLBuffer = (pszFileNameCDL) ? (unsigned char *)malloc(unMaxLength * sizeof(unsigned char)) : NULL;
	char szResultString[0x8000];
	unsigned short unReadSize = 0;

	if (!pucBankBuffer)
		return 0;

	FILE *fpROM = fopen(pszFileNameBank, "rb");
	FILE *fpCDL = (pucCDLBuffer) ? fopen(pszFileNameCDL, "rb") : NULL;
	FILE *fpASM = (pszFileNameASM) ? fopen(pszFileNameASM, "w") : fopen("output.asm", "w");

	for (int i = 0; i < unMaxLength; i++)
		{
		fread(&pucBankBuffer[i], 1, 1, fpROM);
		if (pucCDLBuffer)
			fread(&pucCDLBuffer[i], 1, 1, fpCDL);
		}

	fclose(fpROM);
	fclose(fpCDL);

	printf(".org $%02X\n", unMappedBaseAddress);
	fprintf(fpASM, ".org $%02X\n", unMappedBaseAddress);

	while ((unReadSize += DisassembleSingleInstruction(szResultString, pucBankBuffer, pucCDLBuffer, unReadSize, 0x4000, unMappedBaseAddress)) < unMaxLength)
		{
		printf("%s\n", szResultString);
		fprintf(fpASM, "%s\n", szResultString);
		}
	//need this to print out the last instruction since the above loop stops early
	printf("%s\n", szResultString);
	fprintf(fpASM, "%s\n", szResultString);

	fclose(fpASM);

	return 0;
	}
*/

void main(void)
	{
	unsigned char ucPRGDump[0x4000];
	unsigned char ucCDLDump[0x4000];
	char szResultString[0x400];
	unsigned short unReadSize = 0;
	unsigned short unInstructionSize = 0;

	FILE *fpROM = fopen("d:/projects/crystalis/crystalis.nes", "rb");
	fseek(fpROM, 0x10, SEEK_SET);

	FILE *fpCDL = fopen("d:/projects/crystalis/crystalis.cdl", "rb");

	FILE *fpASM;
	char szASMFileName[0x100];

	unsigned short unMappedBaseAddress = 0x0000; //need to specify 0x8000, 0xC000, etc.

	//prg first
	for (int prgbank = 0; prgbank < 16; prgbank++)
		{
		sprintf(szASMFileName, "d:/projects/crystalis/disassembly/crystalis_prg_0x%01X.asm", prgbank);
		fpASM = fopen(szASMFileName, "w");

		printf(";PRG Bank $%01X\n", prgbank);
		fprintf(fpASM, ";PRG Bank $%01X\n", prgbank);

		printf(".segment \"PRG_0x%01X\"\n", prgbank);
		fprintf(fpASM, ".segment \"PRG_0x%01X\"\n", prgbank);

		//printf(".org $%01X\n", unMappedBaseAddress);
		//fprintf(fpASM, ".org $%01X\n", unMappedBaseAddress);

		printf(".org $8000\n");
		fprintf(fpASM, ".org $8000\n");

		for (int i = 0; i < 0x4000; i++)
			{
			fread(&ucPRGDump[i], 1, 1, fpROM);
			fread(&ucCDLDump[i], 1, 1, fpCDL);
			}

		unReadSize = 0;
		unInstructionSize = 0;

		//prepopulate labels
		int nCurrentAddr = 0;
		int ucOpcodeIndex = 0;
		bool bFound = false;
		int ucOpcode = 0;
		int ucRelativeOperand = 0;

		for (int i = 0; i < 0x4000; i++)
			g_ucIsLabel[i] = 0;

		while (nCurrentAddr < 0x4000)
			{
			bFound = false;
			ucOpcode = ucPRGDump[nCurrentAddr++];

			if (IsCDLVerifiedCode(ucCDLDump[nCurrentAddr - 1]))
				{
				//look to see if the opcode matches the instruction set
				for (int i = 0; i < NUM_OPCODES; i++)
					{
					if (ucOpcode == _Opcodes[i].ucByte)
						{
						ucOpcodeIndex = i;
						bFound = true;
						}
					}

				if (bFound)
					{
					int unDisplacement;

					switch (_Opcodes[ucOpcodeIndex]._enAddressingMode)
						{
						case IMMEDIATE:
						case ZERO_PAGE:
						case ZERO_PAGE_INDEXED_X:
						case ZERO_PAGE_INDEXED_Y:
						case INDEXED_INDIRECT_X:
						case INDIRECT_INDEXED_Y:
							nCurrentAddr++;
							break;
						case ABSOLUTE:
						case INDIRECT_ABSOLUTE:
						case ABSOLUTE_INDEXED_X:
						case ABSOLUTE_INDEXED_Y:
							nCurrentAddr += 2;
							break;
						case RELATIVE:
							ucRelativeOperand = ucPRGDump[nCurrentAddr++];
							unDisplacement = 0;

							if (ucRelativeOperand > 0x7Fu)
								unDisplacement -= ((~ucRelativeOperand & 0x7FU) + 1);
							else
								unDisplacement += ucRelativeOperand & 0x7Fu;

							g_ucIsLabel[unDisplacement + nCurrentAddr] = true;
							break;
						case IMPLIED:
						case ACCUMULATOR:
						default:
							break;
						}
					}
				}
			}
		//done prepopulating labels

		while (unReadSize < 0x4000)
			{
			unInstructionSize = DisassembleSingleInstruction(szResultString, ucPRGDump, ucCDLDump, unReadSize, 0x4000, unMappedBaseAddress, prgbank, -1);
			unReadSize += unInstructionSize;

			//printf("%s\n", szResultString);
			fprintf(fpASM, "%s\n", szResultString);
			}

		fclose(fpASM);
		}

	//now chr
	for (int chrbank = 0; chrbank < 16; chrbank++)
		{
		sprintf(szASMFileName, "d:/projects/crystalis/disassembly/crystalis_chr_0x%01X.asm", chrbank);
		fpASM = fopen(szASMFileName, "w");

		printf(";CHR Bank $%01X\n", chrbank);
		fprintf(fpASM, ";CHR Bank $%01X\n", chrbank);

		printf(".segment \"CHR_0x%01X\"\n", chrbank);
		fprintf(fpASM, ".segment \"CHR_0x%01X\"\n", chrbank);

		//printf(".org $%01X\n", unMappedBaseAddress);
		//fprintf(fpASM, ".org $%01X\n", unMappedBaseAddress);

		printf(".org $8000\n");
		fprintf(fpASM, ".org $8000\n");

		for (int i = 0; i < 0x2000; i++)
			{
			fread(&ucPRGDump[i], 1, 1, fpROM);
			fread(&ucCDLDump[i], 1, 1, fpCDL);
			}

		unReadSize = 0;
		unInstructionSize = 0;

		//prepopulate labels

		int nCurrentAddr = 0;
		int ucOpcodeIndex = 0;
		bool bFound = false;
		int ucOpcode = 0;
		int ucRelativeOperand = 0;

		for (int i = 0; i < 0x2000; i++)
			g_ucIsLabel[i] = 0;

		//dont actually populate labels for chr banks
		/*
		while (nCurrentAddr < 0x2000)
			{
			bFound = false;
			ucOpcode = ucPRGDump[nCurrentAddr++];

			if (IsCDLVerifiedCode(ucCDLDump[nCurrentAddr - 1]))
				{
				//look to see if the opcode matches the instruction set
				for (int i = 0; i < NUM_OPCODES; i++)
					{
					if (ucOpcode == _Opcodes[i].ucByte)
						{
						ucOpcodeIndex = i;
						bFound = true;
						}
					}

				if (bFound)
					{
					int unDisplacement;

					switch (_Opcodes[ucOpcodeIndex]._enAddressingMode)
						{
						case IMMEDIATE:
						case ZERO_PAGE:
						case ZERO_PAGE_INDEXED_X:
						case ZERO_PAGE_INDEXED_Y:
						case INDEXED_INDIRECT_X:
						case INDIRECT_INDEXED_Y:
							nCurrentAddr++;
							break;
						case ABSOLUTE:
						case INDIRECT_ABSOLUTE:
						case ABSOLUTE_INDEXED_X:
						case ABSOLUTE_INDEXED_Y:
							nCurrentAddr += 2;
							break;
						case RELATIVE:
							ucRelativeOperand = ucPRGDump[nCurrentAddr++];
							unDisplacement = 0;

							if (ucRelativeOperand > 0x7Fu)
								unDisplacement -= ((~ucRelativeOperand & 0x7FU) + 1);
							else
								unDisplacement += ucRelativeOperand & 0x7Fu;

							g_ucIsLabel[unDisplacement + nCurrentAddr] = true;
							break;
						case IMPLIED:
						case ACCUMULATOR:
						default:
							break;
						}
					}
				}
			else
				nCurrentAddr++;
			}
		*/
		//done prepopulating labels

		while (unReadSize < 0x2000)
			{
			unInstructionSize = DisassembleSingleInstruction(szResultString, ucPRGDump, ucCDLDump, unReadSize, 0x2000, unMappedBaseAddress, -1, chrbank);
			unReadSize += unInstructionSize;

			//printf("%s\n", szResultString);
			fprintf(fpASM, "%s\n", szResultString);
			}

		fclose(fpASM);
		}

	fclose(fpROM);
	fclose(fpCDL);

	int j;
	scanf("%d", &j);
	}