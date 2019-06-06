#define _CRT_SECURE_NO_WARNINGS 1

#include "stdlib.h"
#include "string.h"

#include "Instruction.h"

void PopulateInstructionString(__InstructionString *p_InstructionString, unsigned short nInstructionLength, unsigned short nBaseAddress, unsigned short nOffsetAddress, char *pszDisassemblyString)
	{
	p_InstructionString->nBaseAddress = nBaseAddress;
	p_InstructionString->nOffsetAddress = nOffsetAddress;
	p_InstructionString->nInstructionLength = nInstructionLength;

	if (pszDisassemblyString != NULL)
		{
		p_InstructionString->pszDisassemblyString = (char *)malloc(sizeof(char) * strlen(p_InstructionString->pszDisassemblyString));
		strcpy(p_InstructionString->pszDisassemblyString, pszDisassemblyString);
		}
	}