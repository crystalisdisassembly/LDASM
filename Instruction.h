#pragma once

typedef struct __InstructionString
	{
	unsigned short nBaseAddress;
	unsigned short nOffsetAddress;
	unsigned short nInstructionLength;

	char *pszDisassemblyString;
	}__InstructionString;

void PopulateInstructionString(__InstructionString *p_InstructionString, unsigned short nInstructionLength, unsigned short nBaseAddress, unsigned short nOffsetAddress, char *pszDisassemblyString);