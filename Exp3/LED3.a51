P4 EQU 0C0H
P4SW EQU 0BBH
ORG 0000H
	LJMP START
ORG 0040H
LEDTABLE:	DB 0C0H
			DB 0F9H
			DB 0A4H
			DB 0B0H
			DB 099H
			DB 092H
			DB 082H
			DB 0F8H
			DB 080H
			DB 090H
			DB 088H
			DB 083H
			DB 0C6H
			DB 0A1H
			DB 086H
			DB 08EH
START:
	MOV DPTR, #LEDTABLE

	MOV P4SW, #00110000B
	
	MOV R0, #00H
	MOV R1, #00H
	MOV R2, #00H
	
ENTRY:

	MOV A, P3
	ANL A, #01000000B
	CJNE A, #01000000B, ENTRY

	ACALL DELAY
	
	INC R0
	CJNE R0, #10, DISP
	
	MOV R0, #00H
	INC R1
	CJNE R1, #10, DISP
	
	MOV R1, #00H
	INC R2
	CJNE R2, #10, DISP
	
	JMP START

DISP:
	ACALL LEDDISPLAY
	
	JMP ENTRY
	
	
DELAY:
	;PUSH R7
	MOV R5, #5
	MOV R6, #255
	MOV R7, #255
DL:
	DJNZ R7, DL
	DJNZ R6, DL
	DJNZ R5, DL
	;POP R7
	RET

LEDDISPLAY:

	MOV A, R0
	MOVC A, @A+DPTR
	
	ACALL DISPLAYBYREG
	
	MOV A, R1
	MOVC A, @A+DPTR
	
	ACALL DISPLAYBYREG
	
	MOV A, R2
	MOVC A, @A+DPTR
	
	ACALL DISPLAYBYREG

	RET

DISPLAYBYREG:
	
	MOV R7, #08H
SHIFTOUT:
	MOV P0, A
	RLC A
	;SHIFT REGISTER
	MOV P4.5, C	;BIT DATA
	CLR P4.4
	SETB P4.4	;CLOCK
	
	DJNZ R7, SHIFTOUT
	
	RET
END