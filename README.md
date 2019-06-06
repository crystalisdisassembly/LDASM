# LDASM
A literal disassembler for the NES game crystalis (to produce output that reassembles with ca65). It works for what I need it to but it's not pretty. There's probably better alternatives, but I enjoyed writing this. The good parts are heavily adapted from dcc6502 (https://github.com/tcarmelveilleux/dcc6502/blob/master/dcc6502.c). If you're here, you might also find my crystalisdisassembly repo (https://github.com/crystalisdisassembly/crystalisdisassembly) interesting.

Note: I do not really maintain this repo, but in case it's of interest to you, the files are here, and should be buildable with visual studio.

This repo is a very hacked together disassembler I wrote to produce reassemblable output (compatible with ca65). It uses CDL logs from Mesen (FCEUX should also work, I think) to differentiate code from data, but there are a few caveats:

1. Mesen's CDL seems to sometimes be an approximation. Mesen flags certain areas of the ROM as both code and data and I'm not sure if this is always accurate. There's a fair amount of poorly legible output including quite a few "invalid opcode" comments in the .asm files, which is a consequence of the ambiguous CDL.
2. The above issue prevents the CHR banks from reassembling. My disassembler therefore treats all ambiguous bytes in the CHR (anything marked as both code and data in the CDL) as data and outputs it via ".byte" commands.
3. There's also ambiguous data in the PRG banks but this doesn't interfere with reassembly and is therefore treated as code.
4. There's a single problem instruction in PRG bank 0x8. Offset 0x29B7 disassembles into "BCC #$01", a branch to 0x29BA. However, this isn't aligned with the disassembly, as offset 0x29B9 contains the instruction JMP $A9C9 and the BCC would move the program counter to the operand, rather than the JMP instruction itself. I don't understand the behavior of this code to understand why it's written this way but regardless, ca65 won't accept an immediate operand for the BCC instruction. Therefore the BCC #$01 is emitted as ".byte $90, $01".

My most recent CDL is available at the crystalisdisassembly repo.
