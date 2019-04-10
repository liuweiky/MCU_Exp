P4 EQU 0C0H
P4SW EQU 0BBH
;INITIAL S
SPEED60SH EQU 05DH
SPEED60SL EQU 03DH

;COUNTER SOFTWARE ASSISTANT
CLANT EQU R3
SWCOUNTER EQU R4
STATE EQU R5
DELAYR EQU R6

ORG 0000H
	LJMP START
ORG 000BH
	LJMP T0IN
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

	MOV P4SW, #00110000B

	MOV DPTR, #LEDTABLE
	MOV R0, #00H
	MOV R1, #00H
	MOV R2, #00H
	
	MOV TMOD, #11110001B
	MOV TL0, #SPEED60SL
	MOV TH0, #SPEED60SH
	MOV TCON, #00010000B
	
	MOV SWCOUNTER, #50
	MOV CLANT, #1
	MOV DELAYR, #0
	
	MOV IE, #10000010B	;T0 INTERRUPT ENABLE
	MOV IP, #00000010B	;T0	INTERRUPT PRIORITY


	JMP $
	
T0IN:
	;JNB P3.6, HIGHSPD
	;;S1 = 1, LOW SPEED
	;DJNZ SWCOUNTER, INTRET
	;MOV SWCOUNTER, #6
;HIGHSPD:
	;;S1 = 0, HIGH SPEED
	
	MOV A, DELAYR
	JNZ INTRET
	
	DJNZ SWCOUNTER, INTRET
	MOV SWCOUNTER, #50
	MOV DELAYR, #50
	
	MOV A, CLANT
	CJNE A, #1, REV
	MOV CLANT, #0
	AJMP INTRET
	
REV:
	MOV CLANT, #1
	AJMP INTRET
	
INTRET:
	ACALL NEXTSTEP
	MOV TL0, #SPEED60SL
	MOV TH0, #SPEED60SH
	RETI

NEXTSTEP:
	
	MOV A, DELAYR
	JZ NEX
	DEC DELAYR
	AJMP NEXTRET
	
NEX:
	CLR P1.1
	CLR P1.4
	
	MOV A, CLANT
	JZ ANTI	;S2
	
	;S2 = 0, CLOCKWISE
	
	CJNE STATE, #0, NNN1
	;00->10
	MOV STATE, #2
	AJMP S10

NNN1:
	CJNE STATE, #1, NNN2
	;01->00
	MOV STATE, #0
	AJMP S00
	
NNN2:
	CJNE STATE, #2, NNN3
	;10->11
	MOV STATE, #3
	AJMP S11

NNN3:
	;CJNE STATE, #3, NNN3
	;11->01
	MOV STATE, #1
	AJMP S01
	
ANTI:
	;S2 = 1, ANTICLOCKWISE
	
	CJNE STATE, #0, ANT1
	;00->01
	MOV STATE, #1
	AJMP S01

ANT1:
	CJNE STATE, #1, ANT2
	;01->11
	MOV STATE, #3
	AJMP S11
	
ANT2:
	CJNE STATE, #2, ANT3
	;10->00
	MOV STATE, #0
	AJMP S00

ANT3:
	;CJNE STATE, #3, ANT3
	;11->10
	MOV STATE, #2
	AJMP S10

S00:
	CLR P3.2
	CLR P1.0
	AJMP NEXTRET
S01:
	CLR P3.2
	SETB P1.0
	AJMP NEXTRET
S10:
	SETB P3.2
	CLR P1.0
	AJMP NEXTRET
S11:
	SETB P3.2
	SETB P1.0
	AJMP NEXTRET
	
NEXTRET:
	SETB P1.1
	SETB P1.4
	ACALL LEDUPDATE
	RET

LEDUPDATE:
	INC R0
	CJNE R0, #10, DISP
	
	MOV R0, #00H
	INC R1
	CJNE R1, #10, DISP
	
	MOV R1, #00H
	INC R2
	CJNE R2, #10, DISP
	
	MOV R0, #00H
	MOV R1, #00H
	MOV R2, #00H

DISP:
	ACALL LEDDISPLAY
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
	
	RLC A
	;SHIFT REGISTER
	MOV P4.5, C	;BIT DATA
	CLR P4.4
	SETB P4.4	;CLOCK
	
	DJNZ R7, SHIFTOUT
	
	RET

END