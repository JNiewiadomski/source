	  PAGE 63,132
	  TITLE BASIC SUBROUTINE I/O DRIVER (PDMA32.ASM) ver 1.10  7/21/88

	  SUBTTL PDMA-32 DRIVER DESCRIPTION

	  COMMENT \


		       COPYRIGHT 1988
		     MetraByte Corporation
		     440 Myles Standish Boulevard
		     Taunton, MA. 02780
		     Phone: [617]-880-3000
			    [508]-880-3000 after 7/16/88

-------------------------------------------------------------------------------
Software developed by:-
		     B.W. Phillips
		     MetraByte Corporation
-------------------------------------------------------------------------------

.................... MODIFICATION RECORD ......................................

Version 1.00:      Initial release 4/27/88
		   Note this is an expanded version of the original PDMA-16
		   driver and is functionally similar although there are
		   some differences. To facilitate comparison, these areas are
		   marked ** ------ **.

-------------------------------------------------------------------------------
Version 1.10:      Mode 1 modified to decrement transfer count by 1 before
		   loading the 8237's. This compensates for the extra transfer
		   that an 8237 always makes i.e. when loaded with N makes
		   N + 1 transfers.

-------------------------------------------------------------------------------

.............. EXAMPLE OF SET UP OF CALL TO PDMA-32 BOARD .....................

   There are 2 entry points to this driver. The first entry point, BASENT, is
for CALLS made from Microsoft BASIC (IBM BASICA, GW-BASIC etc.) or compiled
BASIC (IBM BASIC COMPILER, Microsoft QuickBasic etc.). The second entry point,
GENENT (general entry) is for assembly language linkage and also can be
modified for C, Fortran, Pascal etc. The following description applies to
BASIC, for GENENT see the description prior to the label.

   All variables used in the CALL must be initialized prior to entry of the
CALL. The choice of variable names is arbitrary as long as they are referenced
in the CALL in the correct sequence and are of integer type. Use of non-integer
variables will cause strange effects.

   LINE NUMBER (xxx10 etc. denotes BASIC line number)
   -----------
    xxx10  DEF SEG = &5000      'Segment to load subroutine (typical example)
    xxx20  BLOAD "PDMA32.BIN",0 'Load subroutine, ** Note PDMA32 **
    xxx30  MD% = 0 thru 8       'Mode is selected as follows:-

				     MD%=0 : Initialize driver & test
					=1 : Setup D.M.A. transfer
					=2 : Return status
					=3 : Set timer rate
					=4 : Digital output
					=5 : Digital input
					=6 : Auxiliary output
					=7 : Setup & enable interrupt
					=8 : Disable interrupt

    xxx40 D%(0) = ?	    'D%(0) - data appropriate to mode
    xxx50 FLAG% = 0	    'error return flag
    xxx60 PDMA32 = 0	   'load offset, ** Note PDMA32 **

    xxx70 CALL PDMA32 (MD%, D%(0), FLAG%)   '** Note public name is PDMA32 **

Note that the data variable D%(0) is a multi-element integer array.

    Assembly language link format from BASIC(A) is as follows:-

    The number of variable pointers which are passed by this call is 3.
The BP register points to the starting location in the stack (SP reg.)
of the variable pointers. The first instruction after the initial JMP
is to save the BP register for returning to the BASIC(A) program. On
entry, the CS register is set to the subroutine segment (as specified
by DEF SEG) and the DS and SS-SP registers are set to BASIC's data area
and stack.

    The offset into the stack of any one particular variable pointer is
calculated using the equation:-
      
	VAR. POINTER LOCATION = 2 * (n - m) + 6
	  where n is the total number of variables passed (3)
		m is the position of the variable in the CALL list
		     CALL PDMA32 (1, 2, 3 ) 

    Error codes are returned in the FLAG% variable as follows:-

	   CODE
	   ----
	    0 :  No error, O.K.
	    1 :  Driver not initialized
	    2 :  Mode number <0 or >8
	    3 :  Invalid base address, <256 or >1008  **
	    4 :  Hardware error
	    5 :  DMA level illegal, <0, 4, or >7  **
	    6 :  DMA page wraparound error
	    7 :  DMA data direction not 0 or 1
	    8 :  Interrupt level illegal <3, 8, 13 or >15  **
	    9 :  Interrupt source out of range, <0 or >2
	    10:  Interrupt slope not 0 or 1
	    11:  Auto-recycle not 0 or 1
	    12:  Byte or word specifier not 0 or 1
	    13:  Digital output data out of range for byte output, <0 or >255
	    14:  Port configuration code out of range, <0 or >2
	    15:  Configuration change data not 0 or 1
	    16:  Auxiliary output data out of range
	    17:  Transfer clock source data illegal, not 0 or 1
	    18:
	    19:

e.c.\

	PAGE

	SUBTTL PDMA-32 CALL ROUTINES

	extrn	@pdma32_interrupt$qv:far ;function called by interrupt routine

	CSEG SEGMENT BYTE 'CODE'
;	PUBLIC PDMA32			;** New public name **
;	PDMA32 PROC FAR
;************************************************************
;* keep spirit of CPP alive with special naming conventions *
;************************************************************
	public	@pdma32$qini

@pdma32$qini proc far

	ASSUME CS:CSEG


BASENT:	JMP short START			;entry point from CALL 

	COMMENT \

The following variables are used by the PDMA32.BIN routine and initialized as
shown. They are declared at the start so that the 2 pass Macro-assembler
will treat them correctly as variables and allow use of subsequent segment
overides.

e.c.\

AUTO	DW 0000H		;DMA auto-recycle flag
BASE	DW 0000H		;PDMA32 base I/O address
BASIC	DW 0000H		;BASIC's data segment
BOW	DW 0000H		;byte or word transfer mode
CD0	DW 0FFFFH		;Counter 0 load data
CD1	DW 0FFFFH		;Counter 1 load data
D0	DW 0000H		;data variable 0
D1	DW 0000H		;data variable 1
D2	DW 0000H		;data variable 2
D3	DW 0000H		;data variable 3
D4	DW 0000H		;data variable 4
D5	DW 0000H		;data variable 5
D6	DW 0000H		;data variable 6
DMAFLG	DW 0000H		;DMA active flag
DPTR	DW 0000H		;data array pointer
DPTR_SG	DW 0000H		;data array segment pointer
EFLAG	DW 0000H		;error flag data
EFPTR	DW 0000H		;error flag variable pointer
INITFLG	DW 0000H		;driver initialization flag
INTFLG  DW 0000H		;interrupt active/disabled flag
MODE	DW 0000H		;mode number data
TADD	DW 0000H		;DMA transfer buffer address
TCNT	DW 0FFFFH		;DMA transfer count
TOFF	DW 0000H		;DMA transfer buffer offset
TPAGE	DW 000FH		;DMA transfer buffer page

inum	db 00h			;interrupt number of vector we swapped
iseg	dw 0000h		;old interrupt segment
ioff	dw 0000h		;old interrupt offset


START:	PUSH BP			;save base pointer
	MOV BP,SP		;BP points to stack
	PUSH ES			;save extra-segment
	PUSH BX			;    BX
	PUSH SS			;    and stack segment

	MOV CX,DS		;CX = BASIC's data seg.
	MOV CS:BASIC,CX		;store BASIC's data seg.

;	MOV DI,[BP]+10		;setup pointer to mode data
;	MOV AX,[DI]		;get mode data (call by reference)
;***********************************************************
;* parameters passed in C are in opposite order from BASIC *
;***********************************************************
	mov ax,[bp]+6		;get 1st parameter (call by value)
	MOV CS:MODE,AX		;save data in MODE

;	MOV SI,[BP]+8		;SI = pointer to D%(0) array
;**********************************
;* get far pointer to D%(0) array *
;**********************************
	lds si,dword ptr [bp+8]	;DS:SI = pointer to D%(0) array
	push ds			;prepare to save ds
	pop cs:DPTR_SG		;save segment in DPTR_SG
	MOV CS:DPTR,SI		;store data array location
	MOV DI,OFFSET D0	;DI = pointer to D0-6 local storage
	CLD			;setup for block move, increment
	MOV CX,CS
	MOV ES,CX		;setup ES destination segment
	MOV CX,0007H		;word count
    REP MOVSW			;move array data D0 - D6

;*********************************
;* in C we will return the error *
;*********************************
;	MOV DI,[BP]+6		;DI = pointer to FLAG%
;	MOV CS:EFPTR,DI		;store flag variable location


	PAGE

;------ MODE DESPATCHER ---------------------------------------------------

	COMMENT \

Now that all entry data  has  been  locally  stored,  the  appropriate  routine
can be selected according  to the mode  number specified.  The  mode despatcher
looks after this selection. The mode number is checked to  be  within the range
0 - 8 and an error exit takes place if not.  If  the  driver has not previously
been initialized (INITFLG = 0) and a mode number other than 0 has been selected
an error exit is made.  This  avoids  the  problem  of  selecting modes with an
unknown  I/O address, DMA level, interrupt level etc.  The  final  function  of
the despatcher is to transfer execution to the appropriate mode.

Any error code is  placed  in  the EFLAG local variable for subsequent transfer
on exit from the driver.

Assembly language programmers and  linkages  from  other languages should
first initialize local variables MODE and D0-D6 which are images of the BASIC
variables MD% and D%(0) thru D%(6) and enter at this point (GENENT:). For 
assembly language programmers this is straightforward. If you are working
with a high level language such as Fortran, Pascal, C etc. the stack unloading
section which precedes this comment should be modified to transfer data 
to the local variables. The BASIC version above is a good starting point, 
though most other languages will pass segment:offset pointers on the stack
unlike BASIC which only passes offsets. The order of parameter passing may
also be reversed with some other languages. All of the modules MD_0-8
exchange data only with local variables MODE, D0-D6 and EFLAG as well as the
other temporary local variables. They do not pass data directly to external
program variables, this is the function of the IORET: routine.

e.c.\

GENENT: PUSH CS
	POP DS			;from hereon DS = CS 

	MOV CS:EFLAG,0002H	;err.#2 mode number out of range
	MOV AX,CS:MODE		;fetch mode number
	CMP AX,0000H		;? mode # <0
	JL ERREX		;yes, exit
	CMP AX,0008H		;? mode # >8
	JG ERREX		;yes, exit

	CMP CS:INITFLG,0001H	;driver previously initialized?
	JE MDESP		;yes, despatch mode
	MOV CS:EFLAG,0001H	;err.#1 driver un-initialized
	CMP AX,0000H		;mode 0 selected?
	JE MDESP		;yes, carry on & despatch
ERREX:	JMP short IORET		;error return

MDESP:	SAL AX,1		;2 * mode # for jump table selection
	MOV BX,OFFSET MTABLE	;get start of mode jump table
	ADD AX,BX		;add on 2 * mode #
	MOV SI,AX		;set up relative jump
	MOV AX,[SI]		;get jump address
	JMP AX			;jump to specified mode

;Mode jump table follows:-

MTABLE:	DW MD_0			;initialize driver
	DW MD_1			;setup DMA in/out
	DW MD_2			;return status
	DW MD_3			;set timer rate
	DW MD_4			;digital output
	DW MD_5			;digital input
	DW MD_6			;auxiliary output
	DW MD_7			;interrupt enable 
	DW MD_8			;interrupt disable


	PAGE

;------ COMMON I/O RETURN TO CALLER ROUTINES (Error & Normal) ------------

	COMMENT /

This is a common exit routine (both error and normal) that is set up to
transfer all data back to external BASIC variables. It can be modified to
perform the same function for other languages. On entry to the IORET: label
all the local variables have been updated, so an assembly language programmer
can simply perform a RET or JMP back to their main code at this point.

e.c./

IORET:	MOV SI,OFFSET D0	;SI = pointer to local storage
	MOV DI,CS:DPTR		;DI = pointer to D%(0) array
	CLD			;setup for block move, increment
	MOV ES,CS:BASIC		;setup ES to BASIC's segment
	PUSH ES			;stack BASIC segment
;*************************************************
;* we need to point ES to segment of D%(0) array *
;*************************************************
	push cs:DPTR_SG		;prepare to retrieve segment info
	pop es			;make es point to correct place
	MOV CX,0007H		;word count
    REP MOVSW			;move array data D0 - D6 back to BASIC

	POP DS			;now DS = BASIC's segment
;*********************************
;* we will return the error in C *
;*********************************
;	MOV DI,CS:EFPTR		;get pointer to FLAG%
	MOV AX,CS:EFLAG
;	MOV [DI],AX		;load error code in FLAG%

	POP SS			;restore critical registers
	POP BX
	POP ES
	POP BP
;	RET 6			;return to BASIC caller for 3 arg.
;*********************************************
;* in C, calling routine cleans up the stack *
;*********************************************
	ret			;return to C caller


	PAGE

;------ MODE 0: INITIALIZE DRIVER ----------------------------------------

	COMMENT \

Entry data:-
	      D0 - Base I/O address of PDMA-32, range 100-3F0H
	      D1 - Selected DMA level, 0 - 7 except 4 allowed
	      D2 - Selected interrupt level, 3-7, 9-12 & 14-15 allowed
	      D3 - Selected port mode, 0 = byte, 1 = word, ** see below **
      D4 thru D6 - Not used

Exit data:-
      D0 thru D6 - Unchanged
	    BASE - Set to valid base I/O address
	   EFLAG - 0 if o.k., otherwise:-
		   3 if base address <100H or >3F0H
		   4 if hardware error
		   5 if DMA level illegal
		   8 if interrupt level illegal
		  12 if byte or word specifier, D3, not 0 or 1
	 INITFLG - Set to 1 if initialization successful, 0 otherwise

Mode 0 checks that the base I/O address is in the legal range of 256 - 1008
(Hex 100 - 3F0) for PC/AT's and clones. If not an error exit #3 will occur.
If O.K., the base I/O address is stored in BASE  for  use by other modes on
re-entry to the CALL.

A short read/write test is made to the PDMA-32 DMA and interrupt control
registers and level select registers. This "signature" test is sufficient to
detect the presence or absence of the board at the specified I/O address. If no
board is detected (absent board, wrong type of board, wrong base I/O address),
error #4, hardware error is returned.

** If an invalid DMA level number or interrupt level is selected, errors #5 or
#6 are returned. The active levels are stored in DMA and interrupt level select
registers of the PDMA-32 if valid.  These  registers are also read/write
tested. **

If initialization is successful, the INITFLG is set to 1, allowing selection
of other modes on subsequent CALL's otherwise no mode can be operated with
faulty parameters.

** The PDMA-32 differs from its earlier progenitor, the PDMA-16, in that it
has a 16 bit data bus. This means that for normal I/O transfers, the PA and
PB ports can be treated as byte ports (IN AL,DX & OUT DX,AL) at I/O addresses
BASE and BASE + 1 or a single 16 bit word port (IN AX,DX & OUT DX,AX) at
address BASE. Variable D2 controls whether the board appears to the system
as a 16 bit peripheral on the PA and PB ports, word mode for normal I/O, or
an 8 bit peripheral with 2 independent byte oriented ports PA and PB. All
other I/O ports, BASE + 2 thru 10, are always byte wide regardless of the
operating mode of PA and PB. Note that this applies to programmed I/O only. 
In D.M.A. mode levels 0 - 3 are always byte oriented transfers thru the PA
port and levels 5 - 7 are word oriented thru PA and PB combined. In cases
where the ports are word oriented, the A direction input controls the data
direction of the whole 16 bit port, the B direction input is ignored. It is
necessary to set the operating mode, word or byte, in initialization and
stick to the correct I/O instructions for the mode selected. **

** Note that the PDMA-32 has read/write registers for storing the D.M.A.,
interrupt levels and byte/word mode so these are tested and loaded in
initialization. It is no longer necessary to keep variable images of these
settings (see BOW, DMALEV and INTLEV of the PDMA-16 driver) **

e.c.\

;8254 counter-timer control words:-

CW0	EQU 34H			;control word for counter 0
CW1	EQU 74H			;control word for counter 1
CW2	EQU 0B4H		;control word for counter 2

MD_0:   MOV INITFLG,0		;clear initialization flag
	MOV EFLAG,3		;error #3, base address <100H or >3F0H
	MOV AX,CS:D0		;fetch base address
	CMP AX,0100H		;0FFH or lower?
	JL EX0			;yes, base addr. out of range
	CMP AX,03F0H		;greater than 3F0?
	JG EX0			;yes, out of range
	MOV BASE,AX		;store base address
	MOV DX,AX		;transfer base address to DX

;Simple hardware tests, these tests do not change the state of the AUX port
;output bits or port data or direction on re-initialization. D.M.A. and
;interrupts are disabled.

;** All the following tests pass a good PDMA-32 and will fail on a PDMA-16 **
;If you don't know which board is in use, load the PDMA32.BIN driver first,
;and attempt initialization. If it fails, error #4 returned, load the
;PDMA.BIN driver over the PDMA32.BIN driver and re-attempt initialization.
;If O.K. it must be a PDMA-16. **

	MOV EFLAG,4		;error #4, hardware or addressing error
	INC DX
	INC DX			;base add. + 2 = DMA control
	IN AL,DX		;read control including direction bits

;Check for presence of fixed zero bit (bit 6) in DMA control register
;and disable D.M.A. from PDMA-32.
;Note: DMA control register is always cleared on power up (hardware reset).

	AND AL,03FH		;disable D.M.A.
	MOV AH,AL		;store AL in AH
	OR AL,040H		;set bit 6 high
	CALL DELAY		;PC/AT I/O delay
	OUT DX,AL		;write control
	CALL DELAY		;PC/AT I/O delay
	IN AL,DX		;read back control
	CMP AL,AH		;check if readback o.k. with bit 6 zero
	JNE EX0			;error exit if readback fault

;Check for presence of 3 fixed zero bits in interrupt control register
;and disable interrupts from PDMA-32
;Note: Interrupt control register is always cleared on power up.

	INC DX			;address interrupt control
	IN AL,DX		;read current register state
	AND AL,00FH		;clear upper nybble, bits 4-7
	MOV AH,AL		;and store in AH
	OR AL,070H		;set bits 4-6 high
	CALL DELAY		;PC/AT I/O delay
	OUT DX,AL		;write interrupt control
	CALL DELAY		;PC/AT I/O delay
	IN AL,DX		;read back interrupt control
	CMP AH,AL		;check bits 4-6 returned zero, others no change
	JNE EX0			;error exit if readback fault
	JMP short CKSEL		;jump round error exit point

EX0:	JMP IORET		;error exit

;Check presence and operation of DMA & interrupt level select registers
;These are fully checked with a cyclic code 0-255.

CKSEL:	MOV DX,BASE		;fetch base I/O address
	ADD DX,8		;address D.M.A. select register
STCK:	XOR AX,AX		;clear AX
NXDAT:	OUT DX,AL		;write data
	CALL DELAY		;PC/AT I/O delay
	IN AL,DX		;read back
	CMP AH,AL		;data o.k.?
	JNE EX0			;error exit
	INC AL
	INC AH			;bump AL,AH
	CMP AL,0		;done full cycle?
	JNE NXDAT		;no, continue
	INC DX			;address interrupt select register
	CMP DX,BASE+9
	JE STCK			;if address o.k. do test, else continue

;Clear interrupt flag register

	CALL DELAY		;PC/AT I/O delay
	MOV DX,BASE		;fetch base I/O address
	ADD DX,10		;address interrupt flag register
	IN AL,DX		;read, clears interrupt flag
	CALL DELAY		;PC/AT I/O delay
	IN AL,DX		;2nd. read, this time result must be zero
	CMP AL,0		;? 0
	JNE EX0			;error exit

;Get DMA level:-

	MOV AX,D1		;fetch DMA level
	MOV EFLAG,5		;error #5, DMA level illegal
	CMP AX,0		;is it <0?
	JL EX0			;yes, error exit
	CMP AX,7		;is it >7?
	JG EX0			;yes, error exit
	CMP AX,4		;is it =4, cascade level not on bus?
	JE EX0			;yes, error exit
	MOV DX,BASE		;get base I/O address
	ADD DX,8		;address PDMA-32 D.M.A. register
	OUT DX,AL		;load selected D.M.A. level

;Get interrupt level:-

	MOV AX,D2		;fetch interrupt level
	MOV EFLAG,8		;error #8, interrupt level illegal
	CMP AX,3		;<3 ?
	JL EX0			;yes, error exit
	CMP AX,15		;>15 ?
	JG EX0			;yes, error exit
	CMP AX,8		;=8 ? (not on bus)
	JE EX0			;yes, error exit
	CMP AX,13		;=13 ? (not on bus)
	JE EX0			;yes, error exit
	INC DX			;address PDMA interrupt select register
	OUT DX,AL		;load selected interrupt level

;Load BOW (byte or word mode) variable

	MOV AX,D3		;fetch data
	MOV EFLAG,12		;error #12, byte/word data not 0 or 1
	CMP AX,0		;is it 0?
	JE LBOW			;yes, o.k.
	CMP AX,1		;is it 1?
	JE LBOW			;yes, o.k.

EX00:	JMP IORET		;invalid data, error exit

;Load byte/word mode to correct position in DMA control register

LBOW:	SHL AL,1		;position bit
	SHL AL,1
	MOV AH,AL		;store in AH
	MOV DX,BASE		;fetch base I/O address
	INC DX
	INC DX			;address D.M.A. control register
	IN AL,DX		;read current contents
	CALL DELAY		;PC/AT I/O delay
	AND AL,0FBH		;clear bit 2
	OR AL,AH		;insert byte/word mode data
	OUT DX,AL		;and load D.M.A. control


;Set counter 0 & 1 timer configuration and load. Counters are mode 2 (rate
;generator), double byte load/read, binary.

SETCNT:	MOV DX,BASE		;get base I/O address
	ADD DX,7		;address 8254 control/status
	MOV AL,CW0	      ;control byte for counter 0
	OUT DX,AL		;write byte
	CALL DELAY		;delay for PC/AT
	MOV AL,CW1		;control byte for counter 1
	OUT DX,AL		;write control byte
	CALL DELAY		;delay for PC/AT
	MOV AL,CW2		;control byte for counter 2
	OUT DX,AL		;write control byte
	CALL DELAY		;PC/AT I/O delay
	MOV AL,0FFH		;load counter 1 with 65,535
	DEC DX			;address counter 2
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	OUT DX,AL		;write high byte
	CALL DELAY		;delay for PC/AT
	DEC DX			;address counter 1
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	OUT DX,AL		;write high byte
	CALL DELAY		;delay for PC/AT
	DEC DX			;address counter 0
	OUT DX,AL		;load counter 0 with 65,535
	CALL DELAY		;delay for PC/AT
	OUT DX,AL		;write high byte

;Initialization successful, set flags and return

	MOV EFLAG,0		;clear error flag
	MOV INITFLG,1		;set initialization flag
	JMP IORET		;normal exit


	PAGE

;------ MODE 1: ENABLE DMA IN/OUT, BYTE/WORD --------------------------------

	COMMENT \
Entry data:-
	       D0 - Number of transfers in bytes or words
	       D1 - Value irrelevant
		    ** on PDMA-16 was used to set transfer type, byte/word **
	       D2 - Direction, 0 = input 1 = output
	       D3 - Auto-recycle on/off, 0 = off 1 = on
	       D4 - Transfer clock source, 0 = external 1 = timer
	       D5 - Transfer segment for DMA buffer
	       D6 - Transfer offset for DMA buffer

Exit data:-
       D0 thru D6 - Unchanged
	    EFLAG - 0 if o.k., otherwise:-
		    6 DMA page wrap around error
		    7 DMA direction (D2) not 0 or 1
		   11 Auto-recycle (D3) not 0 or 1
		   17 Transfer clock source (D4) not 0 or 1

Mode 1 sets up both the 8237 DMA controller(s) and the PDMA-32 for an input or
output DMA transfer using bytes or words. If DMA levels 0-3 have been selected
in initialization MD_0 then byte transfers are made through port A, port B
remains independently available for programmed I/O. If DMA levels 5-7 have been
selected then word transfers are made through ports A and B combined.

Note if a transfer count of zero (D0 = 0) is supplied, PDMA-32 will make a full
block transfer of 65,536 transfers. This differs from the PDMA-16 which
abandoned making a transfer if the transfer count was zero and returned anÿerror code.

DMA setup involves a number of steps:-

   1. Disable any DMA transfer already in progress.

   2. Check data for valid ranges. 

   3. Check for a page wraparound problem. This can occur if for
      example you wanted to transfer 40,000 bytes starting at an
      offset of 32,000 in a page. The 4 bit DMA page register
      selects 1 of 16 pages of 64Kbytes out of the memory. The
      IBM hardware is not capable of incrementing the page register
      from the 8237 DMA controller when the end of a page is reached.
      Instead DMA will continue at the beginning of the same page
      and may destroy program or data memory data. This condition 
      of page wraparound is checked before enabling the DMA operation.

   4. The next step is to work out the correct control byte for the
      PDMA-32 control register.

   5. The mode, initial address and byte count registers of the
      appropriate level of the 8237(s) are then loaded.

   6. The timer is halted prior to enabling the DMA channel so that
      the first transfer always occurs a fixed delay after setup.

   7. The 8237 mask register(s) are enabled opening the DMA channel.

   8. The timer is brought up to its correct speed.

   9. Return to the caller then takes place, although the DMA
      transfer may be continuing as the rest of the program executes.

To determine the progress of a transfer, use mode 2. 

Note: If D5 = FFFFH which would set the DMA transfer  segment in
      ROM  space,  BASIC's   data   segment   is   automatically
      substituted instead.  This  makes D.M.A.  directly  to  an
      array variable simple (a dangerous practice but convenient)

	      D%(5) = &HFFFF
	      D%(6) = VARPTR(ARRAY%(0))

e.c.\

;Equates for DMA controllers and page register addresses

PAGE0	EQU 87H			;Level 0 page register I/O address
PAGE1	EQU 83H			;Level 1 page register I/O address
PAGE2	EQU 81H			;Level 2 page register I/O address
PAGE3	EQU 82H			;Level 3 page register I/O address
PAGE5	EQU 8BH			;Level 5 page register I/O address
PAGE6	EQU 89H			;Level 6 page register I/O address
PAGE7	EQU 8AH			;Level 7 page register I/O address
BCON	EQU 00H			;8237 byte controller base address, levels 0-3
WCON	EQU 0C0H		;8237 word controller base address, levels 5-7
BMOD	EQU 0BH			;8237 byte controller mode register I/O address
WMOD	EQU 0D6H		;8237 word controller mode register I/O address
BMSK	EQU 0AH			;8237 byte controller mask register single bit
WMSK	EQU 0D4H		;8237 word controller mask register, single bit

;First disable level to be used (may still be active from incomplete transfer)

MD_1:	MOV DX,BASE		;get base I/O address
	ADD DX,8		;address DMA level select register
	IN AL,DX		;get selected DMA level
	AND AL,07H		;mask out lowest 3 data bits
	MOV DX,BMSK		;set up to disable byte controller
	CMP AL,4		;is byte controller selected, levels 0-3?
	JL DISAB		;yes, so disable
	MOV DX,WMSK		;no, must be word controller, levels 5-7
	SUB AL,4		;adjust level to generate mask code
DISAB:	OR AL,04H		;set mask bit (disable)
	AND AL,07H		;clear top 5 bits
	OUT DX,AL		;mask appropriate controller on selected level
	XOR AX,AX		;AX = 0
	MOV DMAFLG,AX		;clear DMA active flag
	JMP short C1D2		;skip round error exit island

EX1:	JMP IORET		;exit if zero transfers requested

;Check input data all within bounds:-

C1D2:	MOV AX,D0		;fetch desired number of transfers
	DEC AX			;bump transfer count -1 to get correct #
	MOV TCNT,AX		;save transfer count
	MOV EFLAG,7		;Error #7, DMA direction data not 0 or 1
	CMP D2,0		;? <0
	JL EX1			;error exit
	CMP D2,1		;? >1
	JG EX1			;error exit
	MOV EFLAG,11		;Error #11, auto-recycle data not 0 or 1
	MOV AX,D3		;fetch auto-recycle data
	CMP AX,0		;? <0
	JL EX1			;error exit
	CMP AX,1		;? >1
	JG EX1			;error exit
	MOV AUTO,AX		;save auto-recycle data
	MOV EFLAG,17		;Error #17, transfer clock data not 0 or 1
	CMP D4,0		;? <0
	JL EX1			;error exit
	CMP D4,1		;? >1
	JG EX1			;error exit

;Work out transfer page (TPAGE) and transfer offset (TOFF) as 20 bit address

	MOV AX,D5		;get transfer segment
	CMP AX,0FFFFH		;does programmer want BASIC's data seg?
	JNE NADDR		;no, normal address
	MOV AX,BASIC		;yes, substitute BASIC's data segment
NADDR:	XOR BX,BX		;clear BX
	SAL AX,1		;shift left, high order bit in carry flag
	ADC BX,0		;move carry into BX
	SAL BX,1		;shift BX
	SAL AX,1		;shift left, high order bit in carry flag
	ADC BX,0		;move carry into BX
	SAL BX,1		;shift BX
	SAL AX,1		;shift left, high order bit in carry flag
	ADC BX,0		;move carry into BX
	SAL BX,1		;shift BX
	SAL AX,1		;shift left, high order bit in carry flag
	ADC BX,0		;move carry into BX
	MOV CX,D6		;get offset
	ADD AX,CX		;sum with shifted segment
	ADC BX,0		;transfer carry to BX
	MOV TPAGE,BX		;save page
	MOV TOFF,AX		;and offset of 20 bit start address

;Now determine whether a byte (levels 0-3) or word transfer (5-7) requested:-

	MOV DX,BASE		;get base I/O address
	ADD DX,8		;address DMA level select register
	IN AL,DX		;get selected DMA level
	AND AL,07H		;mask lowest 3 bits
	CMP AL,4		;is byte controller selected, levels 0-3?
	JL SETBYT		;yes, so do checkout for byte setup

;Checkout transfer address for word setup:-

	MOV EFLAG,19		;error #19, word transfer on odd boundary
	MOV AX,TOFF		;get the offset to AX
	TEST AL,01H		;check for odd offset
	JE WWRAP		;o.k., so check for word wrap
EX11:	JMP IORET		;error exit, odd offset in word mode
WWRAP:	MOV EFLAG,6		;error #6, D.M.A. page wrap around
	MOV AX,TOFF		;get start offset (byte address)
	SHR AX,1		;change start offset into a word address
	TEST TPAGE,01H		;check if low bit of page address is 1
	JZ WWRAP1		;no, so continue
	ADD AX,8000H		;if so, propagate into word address
	AND TPAGE,0FFFEH	;and zero low bit of page address
WWRAP1:	CLC			;clear carry
	MOV TOFF,AX		;save corrected word address
	ADD AX,TCNT		;calculate terminal offset in words
	JNC DOSET		;no overflow, do controller setup
	JMP IORET		;page wrap around error exit in word mode

;Checkout transfer address for byte setup:-

SETBYT:	MOV EFLAG,6		;error #6, D.M.A. page wrap around
	MOV AX,TOFF		;get start offset
	CLC			;clear carry
	ADD AX,TCNT		;calculate terminal offset in words
	JNC DOSET		;o.k., do setup
	JMP IORET		;page wrap around error exit in byte mode

;All data corect, so start PDMA-32 & 8237 controller(s) setup
;Working out PDMA-32 control register byte

DOSET:	MOV DX,BASE		;get PDMA-32 I/O address
	INC DX
	INC DX			;address DMA control register
	IN AL,DX		;read its contents
	AND AL,33H		;zero bits that will be set
	MOV BX,AX		;move AX to BX
	CALL DELAY		;PC/AT I/O delay
	ADD DX,6		;address DMA level select register
	IN AL,DX		;read DMA level register
	AND AL,07H		;mask out level data
	CMP AL,4		;is it a word setup?
	JL CON1			;no, must be byte, continue setup
	OR BL,04H		;set word/byte bit high for word xfer
CON1:	CMP D2,0		;is it an input transfer?
	JE INPUT		;yes, it's input
	OR BL,01H		;no, must be output, set A direction high
	JMP short CON2
INPUT:	AND BL,0FEH		;input, set A direction low
CON2:	CMP D4,1		;is timer the transfer clock source?
	JNE SDAC		;no, must be external
	OR BL,08H		;yes, enable timer source
SDAC:	OR BL,80H		;set DMA enable bit
	PUSH BX			;save DMA control register byte temporarily

;Work out mode register byte for 8237 DMA controller

	MOV DX,BASE		;fetch PDMA-32 base I/O address
	ADD DX,8		;address D.M.A. level select register
	IN AL,DX		;read D.M.A. level
	AND AL,07H		;mask out data bits
	XOR BX,BX		;BX = 0
	MOV BL,AL		;save D.M.A. level in BL
	CMP AL,4		;is level >4?
	JL GMODE		;no
	SUB AL,4		;yes, adjust AL
GMODE:	CMP D2,0		;is it input mode?
	JE IM			;yes
	OR AL,08H		;no, output so set for read transfer
	JMP short ADAUTO		;skip next line
IM:	OR AL,04H		;input, set for write transfer
ADAUTO:	CMP AUTO,0		;is it auto-initialize mode?
	JE SMOD			;no, so set mode
	OR AL,10H		;yes, enable auto-initialize
SMOD:	OR AL,40H		;select single transfer mode
	CMP BL,4		;is it word or byte level
	JL BMODE		;byte, set byte controller mode
	OUT WMOD,AL		;set 8237 word controller mode register
	JMP short WPAGE		;go and write page register
BMODE:	OUT BMOD,AL		;set 8237 byte controller mode register

;Write page data to appropriate DMA page register, and base and byte count to
;8237 controller.

WPAGE:	MOV AX,TPAGE		;fetch page data
	CMP BL,0		;is it level 0?
	JNE P1			;no, try level 1
	OUT PAGE0,AL		;set level 0 page register
P1:	CMP BL,1		;is it level 1?
	JNE P2			;no, try level 2
	OUT PAGE1,AL		;set level 1 page register
P2:	CMP BL,2		;is it level 2?
	JNE P3			;no, try level 3
	OUT PAGE2,AL		;set level 2 page register
P3:	CMP BL,3		;is it level 3?
	JNE P5			;no, try level 5
	OUT PAGE3,AL		;set level 3 page register
P5:	CMP BL,5		;is it level 5?
	JNE P6			;no, try level 6
	OUT PAGE5,AL		;set level 5 page register
P6:	CMP BL,6		;is it level 6?
	JNE P7			;no, try level 7
	OUT PAGE6,AL		;set level 6 page register
P7:	CMP BL,7		;is it level 7?
	JNE TRCNT		;o.k., load transfer count
	OUT PAGE7,AL		;set level 7 page register

TRCNT:	CMP BL,4		;check DMA level
	JL LBCON		;o.k., byte controller
	MOV DX,WCON		;load base address of 8237 word controller
	XOR AX,AX		;AX = 0
	MOV AL,BL		;get D.M.A. level to AL
	SUB AL,4		;adjust level
	SHL AL,1		;x4 for word controller
	SHL AL,1
	ADD DX,AX		;compute I/O address for channel base
	MOV AX,TOFF		;get channel base offset
	OUT DX,AL		;write low byte of base transfer address
	CALL DELAY		;wait for PC/AT I/O
	MOV AL,AH
	OUT DX,AL		;write high byte of base transfer
	CALL DELAY		;I/O delay for PC/AT
	INC DX
	INC DX			;address controller transfer count register
	MOV AX,TCNT		;fetch transfer count
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte
	JMP short STIM		;skip to set timer

LBCON:	MOV DX,BCON		;load base address of 8237 byte controller
	XOR AX,AX		;AX = 0
	MOV AL,BL		;get D.M.A. level to AL
	SHL AL,1		;x2 for byte controller
	ADD DX,AX		;compute I/O address for channel base
	MOV AX,TOFF		;get channel base offset
	OUT DX,AL		;write low byte of base transfer address
	CALL DELAY		;wait for PC/AT I/O
	MOV AL,AH
	OUT DX,AL		;write high byte of base transfer
	CALL DELAY		;I/O delay for PC/AT
	INC DX			;address controller transfer count register
	MOV AX,TCNT		;fetch transfer count
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte

;Temporarily halt timer by setting to slowest rate (1 pulse every 7 minutes)

STIM:	MOV DX,BASE		;get PDMA-32 I/O address
	ADD DX,7		;address timer control
	MOV AL,CW0		;get control byte for counter 0
	OUT DX,AL		;write control
	CALL DELAY		;delay for PC/AT
	MOV AL,CW1		;get control byte for counter 1
	OUT DX,AL		;write control
	DEC DX
	DEC DX			;address counter 1
	CALL DELAY		;I/O delay for PC/AT
	MOV AL,0FFH		;load counter 1 with 65,535
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	OUT DX,AL		;write high byte
	CALL DELAY		;delay for PC/AT
	DEC DX			;address counter 0
	OUT DX,AL		;load counter 0 with 65,535
	CALL DELAY		;delay for PC/AT
	OUT DX,AL		;write high byte

;Now write control byte to PDMA-32 DMA control register

	DEC DX
	DEC DX			;address control register
	POP AX			;get control byte
	PUSH AX			;and return to stack
	OR AL,04H		;temporarily enable timer
	OUT DX,AL		;PDMA-32 control written and timer enabled

;Enable 8237 mask register

	MOV AL,BL		;fetch D.M.A. level
	CMP AL,4		;is it byte level?
	JL SBMSK		;yes, so set 8237 byte controller mask bit
	SUB AL,4		;no, set 8237 word controller mask bit
	OUT WMSK,AL		;write word controller mask
	JMP short RLD			;now re-load

SBMSK:	OUT BMSK,AL		;set 8237 byte controller mask

;8237 D.M.A. controller now active, but there may have been a pending D.M.A.
;request. Reload 8237 byte count and address registers. PDMA-32 timer is hung
;so it will not produce any request.

RLD:	CMP BL,4		;check DMA level
	JL LBCONA		;o.k., byte controller
	MOV DX,WCON		;load base address of 8237 word controller
	XOR AX,AX		;AX = 0
	MOV AL,BL		;get D.M.A. level to AL
	SUB AL,4		;adjust level
	SHL AL,1		;x4 for word controller
	SHL AL,1
	ADD DX,AX		;compute I/O address for channel base
	MOV AX,TOFF		;get channel base offset
	OUT DX,AL		;write low byte of base transfer address
	CALL DELAY		;wait for PC/AT I/O
	MOV AL,AH
	OUT DX,AL		;write high byte of base transfer
	CALL DELAY		;I/O delay for PC/AT
	INC DX
	INC DX			;address controller transfer count register
	MOV AX,TCNT		;fetch transfer count
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte
	JMP short REWC		;skip to re-load PDMA-32 control

LBCONA:	MOV DX,BCON		;load base address of 8237 byte controller
	XOR AX,AX		;AX = 0
	MOV AL,BL		;get D.M.A. level to AL
	SHL AL,1		;x2 for byte controller
	ADD DX,AX		;compute I/O address for channel base
	MOV AX,TOFF		;get channel base offset
	OUT DX,AL		;write low byte of base transfer address
	CALL DELAY		;wait for PC/AT I/O
	MOV AL,AH
	OUT DX,AL		;write high byte of base transfer
	CALL DELAY		;I/O delay for PC/AT
	INC DX			;address controller transfer count register
	MOV AX,TCNT		;fetch transfer count
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte


;Rewrite PDMA-32 control with correct timer or external select

REWC:	MOV DX,BASE		;get PDMA-32 base I/O address
	INC DX
	INC DX			;address PDMA-32 control register
	POP AX			;retrieve control byte
	OUT DX,AL		;now set up control with correct data

;Restore timer to correct rate

	ADD DX,05H		;address counter control
	MOV AL,CW0		;get counter 0 control byte
	OUT DX,AL		;write it
	CALL DELAY		;I/O delay for PC/AT
	MOV AL,CW1		;get counter 1 control byte
	OUT DX,AL		;write it
	CALL DELAY		;I/O delay for PC/AT
	DEC DX
	DEC DX			;address counter 1
	MOV AX,CD1		;fetch counter 1 data
	OUT DX,AL		;write low byte
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte
	CALL DELAY		;delay for PC/AT
	DEC DX			;address counter 0
	MOV AX,CD0		;get counter 0 data
	OUT DX,AL		;load counter 0 
	CALL DELAY		;delay for PC/AT
	MOV AL,AH
	OUT DX,AL		;write high byte

	MOV EFLAG,0		;clear error flag
	MOV DMAFLG,1		;set DMA active flag
	JMP IORET		;return to caller


	PAGE

;------ MODE 2: READ STATUS --------------------------------------------------

	COMMENT \

Entry data:-

       D0 thru D6 - value irrelevant

Exit data:-
	       D0 - Number of bytes/words transferred
	       D1 - Number of byte/word transfers requested
	       D2 - Auto-recycle on/off, 1 = on 0 = off
	       D3 - DMA active/finished, 1 = active 0 = finished
	       D4 - Port A data direction, 0 = input 1 = output
	       D5 - Port B data direction, 0 = input 1 = output
	       D6 - Interrupt status, 0 = disabled 1 = enabled
	    EFLAG - 0 (always o.k.)

Mode 2 returns information about the status of DMA and interrupt operations.
For DMA, D3 indicates whether a DMA transfer set up by mode 1 is still active.
It also returns the number of transfers (bytes or words) requested in mode 1
in D1 and the number transferred so far in D0. If the DMA controller is
operating in non-autoinitialize mode and the transfer is complete (number
transferred = number requested), then the DMAFLG is set to zero, and D3 returns
0 to indicate DMA inactive. It is also possible for the hardware to generate a 
terminal interrupt to indicate the end of a DMA transfer, although this driver
does not utilize this capability.

Mode 2 also returns the current data directions of ports A and B and the
current interrupt status from the interrupt flag, INTFLG which is set on
enabling interrupts with mode 8 and cleared by mode 9. It is possible for
you to set up your interrupt service routine to clear this flag automatically.

e.c.\

MD_2:	MOV DX,BASE		;get PDMA-32 base I/O address
	ADD DX,8		;address DMA level select register
	XOR AX,AX		;AX = 0
	IN AL,DX		;read back level setting
	AND AL,07H		;mask out data
	CMP AL,4		;is it byte or word level?
	JL BYT			;byte level
	SUB AL,4		;adjust level for word controller
	MOV BL,AL		;save in BL
	OR AL,04H		;disable mask bit
	PUSH AX			;save disable code
	MOV DX,WCON		;fetch base address of 8237 word controller
	INC DX
	INC DX			;address channel 4 word count
	AND AL,03H		;mask out channel number
	SHL AL,1
	SHL AL,1		;channel x4
	ADD DX,AX		;DX has address of word count register
	POP AX			;restore mask disable code
	OUT WMSK,AL		;disable D.M.A. while reading registers
	IN AL,DX		;read low byte of count
	CALL DELAY		;I/O delay for PC/AT
	MOV AH,AL
	IN AL,DX		;read high byte
	XCHG AH,AL		;adjust bytes
	MOV CX,AX		;save count in CX
	CMP AUTO,1		;auto-initialize mode?
	JE REW			;yes, so re-enable DMA
	CMP CX,0FFFFH		;no, one shot, final count?
	JE RON			;yes, do not re-enable
REW:	MOV AL,BL		;re-enable D.M.A.
	OUT WMSK,AL
RON:	JMP short RDNO		;read out number

BYT:	MOV BL,AL		;save level in BL
	OR AL,04H		;disable mask bit
	PUSH AX			;save disable code
	MOV DX,BCON		;fetch base address of 8237 byte controller
	INC DX			;address channel 0 word count
	AND AL,03H		;mask out channel number
	SHL AL,1		;channel x2
	ADD DX,AX		;DX has address of word count register
	POP AX			;restore mask disable code
	OUT BMSK,AL		;disable D.M.A. while reading registers
	IN AL,DX		;read low byte of count
	CALL DELAY		;I/O delay for PC/AT
	MOV AH,AL
	IN AL,DX		;read high byte
	XCHG AH,AL		;adjust bytes
	MOV CX,AX		;save count in CX
	CMP AUTO,1		;auto-initialize mode?
	JE BEW			;yes, so re-enable DMA
	CMP CX,0FFFFH		;no, one shot, final count?
	JE RDNO			;yes, do not re-enable
BEW:	MOV AL,BL		;re-enable D.M.A.
	OUT BMSK,AL

RDNO:	CMP CX,0FFFFH		;at last count?
	JNE ROUT		;no
	CMP AUTO,1		;in autoinitialize mode?
	JE ROUT			;yes
	MOV DMAFLG,0		;no, so clear DMA flag
	MOV D3,0		;also clear D3
ROUT:	MOV BX,TCNT		;get transfer count set in mode 1
 	SUB BX,CX		;AX = number words/bytes transferred so far
	MOV D0,BX		;transfer to variables
	MOV AX,TCNT		;restore byte/word count
	INC AX			;correct transfer count
	MOV D1,AX
	MOV AX,AUTO
	MOV D2,AX		;autoinitialize data
	MOV AX,DMAFLG		;fetch DMA flag
	MOV D3,AX		;and transfer DMA status

;Read PDMA-32 control register to get data directions of ports A & B

	MOV DX,BASE		;get PDMA-32 I/O address
	INC DX
	INC DX			;address control
	IN AL,DX		;read it
	PUSH AX			;store it
	AND AX,0001H		;mask out port A data
	MOV D4,AX		;send it to D4
	POP AX
	AND AX,0002H		;mask out port B data
	SHR AX,1
	MOV D5,AX		;send it to D5

;Fetch interrupt status

	MOV AX,INTFLG		;get the interrupt flag
	MOV D6,AX		;and send it to D6

	MOV EFLAG,0		;clear errors
	JMP IORET		;return to caller


	PAGE

;------ MODE 3: SET TIMER RATE  -----------------------------------------------

	COMMENT \
Entry data:-
	       D0 - Counter 0 divider
	       D1 - Counter 1 divider
       D2 thru D6 - not used

Exit data:-
       D0 thru D6 - Unchanged
	    EFLAG - 0 (always o.k.)

Mode 3 sets the division ratio for the timer. The timer output consists
of two 16 bit down counters driven in cascade from a 10.000MHz xtal clock.
Mode 0 initializes these counters to operate in 8254 mode 2 or rate generator 
mode which produces a single pulse on terminal count of counter 1. Mode 0
also initially loads these counters to 65,535 (FFFFH) so that the initialized
output rate is about 1 pulse every 7 minutes. Immediately after initialization
the counter output is guaranteed to be low for 7 minutes which is good for
set up of an external device.

This mode re-loads the counters to any other desired rate up to a maximum of
2.5MHz. The output pulse rate is:-
		  
	  Rate = (10,000,000)/(D0 * D1)   pulses per sec.

Permissible values for D0 and/or D1 are 2 and 4 thru 65,535. Values of 0,1 and 
3 are not legal.

It is also possible to operate the counters in square wave mode (8254 mode 3)
but this requires direct programming to the 8254 control port as this driver
does not support changing the initialized configuration. Counting may also be
enabled and disabled with the TIMER GATE input (pin 3). When held low, this
input inhibits counting, when open or held high counting is allowed to
proceed.

e.c.\

MD_3:	MOV DX,BASE		;get base I/O address
	ADD DX,7		;address counter control
	MOV AL,CW0		;get counter 0 control byte
	OUT DX,AL		;write 8254 control
	DEC DX
	DEC DX
	DEC DX			;address counter 0
	CALL DELAY		;I/O delay for PC/AT
	MOV AX,D0		;get counter 0 load data
	MOV CD0,AX		;and store for DMA setup
	OUT DX,AL		;write low byte counter 0
	MOV AL,AH
	CALL DELAY		;I/O delay for PC/AT
	OUT DX,AL		;write high byte counter 0
	CALL DELAY		;I/O delay for PC/AT
	INC DX			;address control
	INC DX
	INC DX
	MOV AL,CW1		;counter 1 control byte
	OUT DX,AL		;write control
	CALL DELAY		;I/O delay for PC/AT
	DEC DX
	DEC DX			;address counter 1
	MOV AX,D1		;get counter 1 load data
	MOV CD1,AX		;and store for DMA setup
	OUT DX,AL		;write low byte counter 1
	MOV AL,AH
	CALL DELAY		;I/O delay for PC/AT
	OUT DX,AL		;write high byte counter 1

	MOV EFLAG,0		;clear errors
	JMP IORET		;return to caller


	PAGE

;------ MODE 4: WRITE PORT A/B DATA --------------------------------------------

	COMMENT \
Entry data:-
	       D0 - output data, range 0 to 255 if byte
				 -32768 to 32767 if word
	       D1 - output port(s) - value relevant only if ports in byte mode
				 0 = port A (byte)
				 1 = port B (byte)

       D2 thru D6 - not used

Exit data:-
      D0 thru D6 - Unchanged
	   EFLAG - 0 if o.k., otherwise:-
		   13 if data (D0) is out of range, <0 or >255 for byte xfers.
		   14 if port configuration (D1) data <0 or >1

Mode 4 performs digital output as a programmed transfer either through the 
A port or the B port for byte transfers, or in the case of word transfers
through both A & B ports combined. /** The operating mode of the board, whether
ports A & B are treated as bytes or a word is set in initialization mode 0 **/
D1 selects which port data is directed to in the case of byte transfers. If the
PDMA-32 is operating in word mode, then the value of D1 is irrelevant, data
will be directed as a single word to both ports A & B combined. In word mode,
port A outputs the least significant byte and port B outputs the most
significant.

The operation performed by this mode is equivalent to a direct write to the
port address using OUT addr, data in most languages. Once written, data is
latched on the output port and can be read back using mode 5 or INP(addr).


The data direction of the ports is checked initially by reading the PDMA-32's
DMA control register. If it differs from the desired output arrangement, the 
data directions are set accordingly. On power up, all ports are reset to the
input condition.

A check on valid data is performed on byte transfers only and error code #13
is returned if out of range and the output operation abandoned.		

e.c.\

MD_4:	MOV DX,BASE		;get PDMA-32 base I/O address
	INC DX
	INC DX			;address D.M.A. control
	IN AL,DX		;read control
	TEST AL,04H		;check whether in word or byte I/O mode
	JZ BYOUT		;if zero must be byte I/O

;Word output

	TEST AL,01H		;word I/O, is data direction output?
	JNZ CWOUT		;if it is continue
	OR AL,01H		;else set ports A & B to output mode
	CALL DELAY		;PC/AT I/O delay
	OUT DX,AL		;write DMA control
CWOUT:	MOV AX,D0		;fetch data
	DEC DX
	DEC DX			;address ports A & B
	OUT DX,AX		;word output
	MOV EFLAG,0		;clear errors
EX4:	JMP IORET		;return to caller

;Byte output

BYOUT:	MOV EFLAG,13		;error #13, data <0 or >255 (not byte range)
	CMP D0,0		;? <0
	JL EX4			;error exit
	CMP D0,255		;? >255
	JG EX4
	INC EFLAG		;error #14, port select code <0 or >1
	MOV BX,D1		;fetch output port specifier
	CMP BX,0		;? <0
	JL EX4			;yes, error exit
	CMP BX,1		;? >1
	JG EX4			;yes, error exit
	INC BX			;bump BX to get control reg. codes
	MOV DX,BASE		;get PDMA-32 I/O address
	INC DX
	INC DX			;for DMA control register
	IN AL,DX		;read control
	TEST AL,BL		;check if data direction bit A or B set output
	JNZ CBYT		;proceed if no change required
	OR AL,BL	 	;or change configuration to that required
	OUT DX,AL
	CALL DELAY		;PC/AT I/O delay
CBYT:	MOV AX,D0		;fetch data
	DEC DX
	DEC DX			;DX = base address
	ADD DX,D1		;address appropriate port A or B
	OUT DX,AL		;output byte
	MOV EFLAG,0		;clear errors
	JMP IORET		;and return to caller


	PAGE

;------ MODE 5: READ PORT A/B DATA -------------------------------------------

	COMMENT \
Entry data:-
	       D0 - value irrelevant
	       D1 - input port in byte mode, value irrelevant in word mode
				 0 = port A (byte)
				 1 = port B (byte)
	       D2 - change configuration
				 0 = no change
				 1 = change addressed port to input mode
       D2 thru D6 - not used

Exit data:-
	      D0 - data from port
      D1 thru D6 - Unchanged
	   EFLAG - 0 if o.k., otherwise:-
		   14 if port selection (D1) data <0 or >1
		   15 if config. change data (D2) not 0 or 1

Mode 5 reads byte (8 bit) data from port A or port B as specified by D1 if
PDMA-32 has been initialized in byte I/O mode (mode 0), or word (16 bit) data
from ports A and B combined if initialed in word mode. In the case of word data
reads, port A provides the least significant byte, and port B the most
significant. Data is always returned in D0.

Note that the ports may be in input or output mode on entry to mode 5. If D2
is 0, mode 5 will not interfere with the port configuration i.e. will read back
data from a port set in output mode by mode 4 and leave it set in output mode.
If however you want to alter the configuration and make the port an input port
only, D2 should be set to 1. An addressed port will then the be set in input
mode prior to reading it and will remain in input mode afterwards until changed
by an output operation using mode 4. Ports are normally configured as inputs
automatically by the computer power on reset.

e.c.\

MD_5:	MOV EFLAG,15		;error #15, configuration data not 0 or 1
	CMP D2,0		;? <0
	JL EX5			;error exit
	CMP D2,1		;? >1
	JG EX5			;error exit
	MOV DX,BASE		;get PDMA-32 base I/O address
	INC DX
	INC DX			;and set to DMA control register
	IN AL,DX		;read DMA control
	TEST AL,04H		;and check bit 2
	JZ BYIN			;if bit 2 = 1 then byte input I/O

;Word input

	CMP D2,1		;? change configuration to input mode
	JNE CWIN		;no, so continue word input
	AND AL,0FEH		;clear bit 1, set input mode
	CALL DELAY		;PC/AT I/O delay
	OUT DX,AL		;update PDMA-32 control register direction A
	CALL DELAY		;PC/AT I/O delay
CWIN:	DEC DX
	DEC DX			;DX = PDMA-32 base address
	IN AX,DX		;read word from PA-PB ports
	MOV D0,AX		;transfer data to D0
	MOV EFLAG,0		;clear errors
EX5:	JMP IORET		;return to caller

;Byte input

BYIN:	DEC EFLAG		;error #14, port selection data not 0 or 1
	MOV BX,D1		;transfer select data to BX
	CMP BX,0		;? <0
	JL EX5			;error exit
	CMP BX,1		;? >1
	JG EX5			;error exit
	INC BX			;bump BX
	NOT BX			;compliment BX
	CMP D2,1		;? change configuration to input mode
	JNE BIN			;no, so continue byte input
	AND AL,BL		;clear bit 1 or 2, set input mode
	CALL DELAY		;PC/AT I/O delay
	OUT DX,AL		;update PDMA-32 control register direction A
	CALL DELAY		;PC/AT I/O delay
BIN:	DEC DX
	DEC DX			;address port A
	ADD DX,D1		;address selected port (A or B)
	XOR AX,AX		;AX = 0
	IN AL,DX		;read byte input
	MOV D0,AX		;transfer data to D0
	MOV EFLAG,0		;clear errors
	JMP IORET		;return to caller


	PAGE

;------ MODE 6: WRITE DATA TO AUX1-3 -----------------------------------------

	COMMENT \
Entry data:-
	       D0 - AUX 1 data (0 or 1 permitted)
	       D1 - AUX 2 data (0 or 1 permitted)
	       D2 - AUX 3 data (0 or 1 permitted)
       D3 thru D6 - not used

Exit data:-
       D0 thru D6 - Unchanged
	    EFLAG - 0 if o.k., otherwise:-
		    16 if any auxiliary data bit is not 0 or 1
Mode 6 provides a means of writing data to any of the auxiliary data bits
of the DMA or interrupt control registers. Data is OR'ed into the registers
so that other control bits are not changed.

If any of the data in D0 thru D2 is not 0 or 1, error code 16 is returned.

e.c.\

MD_6:	MOV EFLAG,16		;error #16, data not 0 or 1
	CMP D0,0		;? D0 <0
	JL EX6
	CMP D0,1		;? D0 >1
	JG EX6
	CMP D1,0		;? D1 <0
	JL EX6
	CMP D1,1		;? D1 >1
	JG EX6
	CMP D2,0		;? D2 <0
	JL EX6
	CMP D2,1		;? D2 >1
	JG EX6
	JMP short LAUX		;o.k. data valid, load bits

EX6C:	MOV EFLAG,0		;clear error flag
EX6:	JMP IORET		;return to caller

LAUX:	MOV DX,BASE		;get base I/O address
	INC DX
	INC DX			;address DMA control reg.
	IN AL,DX		;read control
	AND AL,0CFH		;zero bits 4 & 5
	MOV BX,D1		;get AUX 2 data
	SAL BX,1
	OR BX,D0		;add AUX 1 data
	MOV CL,4		;prepare for 4 left shifts
	SAL BX,CL
	OR AL,BL		;add new data
	OUT DX,AL		;write new DMA control byte
	CALL DELAY		;PC/AT I/O delay
	INC DX			;address interrupt control
	MOV BX,D2		;get AUX 3 data
	MOV CL,3		;ready for 3 left shifts
	SAL BX,CL
	IN AL,DX		;read interrupt control byte
	AND AL,0F7H		;zero out bit 3
	OR AL,BL		;add new data AUX3
	CALL DELAY		;I/O delay for PC/AT
	OUT DX,AL		;write new interrupt control byte
	JMP EX6C		;clear flag & return to caller


	PAGE

;------ MODE 7: INSTALL AND ENABLE INTERRUPT  ---------------------------------

	COMMENT \
Entry data:-
	       D0 - Interrupt source
				 0 = External input, positive slope
				 1 = External input, negative slope
				 2 = DMA terminal interrupt
				 3 = Timer interrupt
       D1 thru D6 - not used

Exit data:-
       D0 thru D6 - Unchanged
	    EFLAG - 0 if o.k., otherwise:-
		    18 if interrupt source data (D0) out of range, <0 or >3

Mode 7 enables the interrupt on the level set in mode 0 and from the source
selected by D0.

The various steps involved are:-

      1. Check for valid source data

      2. Disable any active interrupt on this level

      3. Install interrupt vectors using DOS function call 25H of INT 21H

      4. Write correct control byte to PDMA-16 interrupt control register

      5. Enable 8259 interrupt mask register

A sample "do-nothing" interrupt service routine is supplied at the end of
this listing (INTH). Users should modify this routine to do whatever they
wish. 

e.c.\

IMR1	EQU 21H		;8259 interrupt controller 1 mask register address
IMR2	EQU 0A1H	;8259 interrupt controller 2 mask register address


MD_7:	MOV EFLAG,18		;interrupt source data out of range <0 or >3
	CMP D0,0		;? <0
	JL EX7			;error exit
	CMP D0,3		;? >3
	JG EX7			;error exit
	JMP short IOFF		;skip next line

EX7:	JMP IORET		;error return to caller

;Disable level to be setup in case it's already active

IOFF:	MOV DX,BASE		;get PDMA-32 base I/O address
	ADD DX,9		;address interrupt select level register
	IN AL,DX		;read selected level
	AND AX,000FH		;mask out bottom 4 data bits
	MOV CX,AX		;transfer level to CX
	PUSH CX			;save CX
	CALL DELAY		;PC/AT I/O delay
	IN AL,IMR1		;read controller 1 mask register
	CMP CX,8		;is it level 9-15 (controller 2)?
	JL IC1			;no, controller 1
	SUB CX,8		;adjust CX for controller 2
	IN AL,IMR2		;read controller 2 mask register
IC1:	MOV BX,0001H		;shift code
	SAL BX,CL		;get OR mask
	OR AL,BL		;set appropriate mask bit
	POP CX			;restore interrupt level to CX
	CMP CX,8		;is it controller 1 or 2?
	JL DO1			;>8 must be controller 1
	OUT IMR2,AL		;must be controller 2, write mask
	JMP short CLINTF

DO1:	OUT IMR1,AL		;controller 1 disable level

CLINTF:	MOV INTFLG,0		;clear interrupt flag

;Setup interrupt vectors using DOS function call 25H

	CMP CX,8		;is it controller 1 or 2?
	JL SV			;<8, controller 1
	ADD CX,0060H		;adjust CX to get vector # for controller 2
SV:	ADD CX,0008H		;adjust CX to get vector # for either controll.
	MOV AL,CL		;AL = hardware vector number
;******************************************************************
;* we want to save previous vector so we can restore it when done *
;******************************************************************
	mov cs:inum,al		;save interrupt number for restore
	mov ah,35h		;dos function to obtain interrupt vector
	int 21h			;call dos
	mov cs:ioff,bx		;save old interrupt vector offset
	mov bx,es
	mov cs:iseg,bx		;save old interrupt vector segment

	mov al,cs:inum		;interrupt number vector we are changing

	MOV AH,25H		;DOS function call 25H - set interrupt vector
	MOV DX,OFFSET INTH	;get offset of interrupt handler
	PUSH CS
	POP DS			;DS:DX contains address of int. handler
	INT 21H			;perform function call

;Build PDMA-32 interrupt control register byte

	MOV DX,BASE		;get PDMA-32 base I/O address
	ADD DX,3		;address interrupt control
	IN AL,DX		;read present contents to preserve AUX3
	OR AL,80H		;set interrupt enable bit
	MOV CX,D0		;get source 
	CMP CX,3		;is it timer?
	JNE ADSRC		;no
	INC CX			;yes, bump to correct code
ADSRC:	OR AX,CX		;AL has interrupt control byte
	OUT DX,AL		;and write to interrupt control

;Enable level in mask register

	MOV DX,BASE		;get PDMA-32 base I/O address
	ADD DX,9		;address interrupt select level register
	IN AL,DX		;read selected level
	AND AX,000FH		;mask out bottom 4 data bits
	MOV CX,AX		;transfer level to CX
	PUSH CX			;save CX
	IN AL,IMR1		;read controller 1 mask register
	CMP CX,8		;is it level 9-15 (controller 2)?
	JL IC2			;no, controller 1
	SUB CX,8		;adjust CX for controller 2
	IN AL,IMR2		;read controller 2 mask register
IC2:	MOV BX,0001H		;shift code
	SAL BX,CL		;get OR mask
	NOT BX			;compliment BX
	AND AL,BL		;clear appropriate mask bit
	POP CX			;restore interrupt level to CX
	CMP CX,8		;is it controller 1 or 2?
	JL CM2			;>8 must be controller 1
	OUT IMR2,AL		;must be controller 2, write mask
	JMP short EX7A

CM2:	OUT IMR1,AL		;write mask on controller 1

EX7A:	MOV INTFLG,1		;set interrupt flag
	MOV EFLAG,0		;clear errors
	JMP IORET		;and return to caller

	PAGE

;------ MODE 8: DISABLE INTERRUPT --------------------------------------------

	COMMENT \
Entry data:-
       D0 thru D6 - value irrelevant

Exit data:-
       D0 thru D6 - Unchanged
	    EFLAG - 0 (always o.k.)

Mode 8 sets the appropriate bit in the appropriate 8259 interrupt controller
1 or 2 mask register to disable further interrupts for the level selected in
mode 0. The INTFLG, interrupt flag, is also cleared to indicate interrupts no
longer active and bit 8 of the PDMA-32 interrupt control register is also
cleared to tri-state the interrupt driver.

No attempt is made to restore previous interrupt routine vectors (possibly
required if sharing an interrupt level). This mode can be expanded to do
this if required.

e.c.\

MD_8:	MOV DX,BASE		;get PDMA-32 base I/O address
	ADD DX,9		;address interrupt level select register
	IN AL,DX		;read selected level
	AND AX,000FH		;mask out interrupt level data
	MOV CX,AX		;transfer interrupt level to CX
	PUSH CX			;save on stack
	CMP CX,8		;is it controller 1 or 2?
	JL MMSK			;make mask bit for controller 1
	SUB CX,8		;adjust CX for mask bit for controller 2
MMSK:	MOV BX,0001H		;shift code
	SAL BX,CL		;prepare OR mask
	POP CX			;restore level
	CMP CX,8		;is it controller 1 or 2?
	JL OFF1			;must be 1
	IN AL,IMR2		;must be 2, read mask
	OR AL,BL		;set mask bit
	CALL DELAY		;PC/AT I/O delay
	OUT IMR2,AL		;disable selected level on controller 2
	JMP short OFF

OFF1:	IN AL,IMR1		;read current mask
	CALL DELAY		;I/O delay for PC/AT
	OR AL,BL		;set appropriate mask bit
	OUT IMR1,AL		;and disable level on controller 1

OFF:	MOV DX,BASE		;get base I/O address
	ADD DX,3		;address interrupt control register
	IN AL,DX		;read current contents
	AND AL,7FH		;clear bit 7
	CALL DELAY		;wait for PC/AT I/O
	OUT DX,AL		;disable & tri-state PDMA-32 driver

;********************************************************
;* we want to restore previously saved interrupt vector *
;********************************************************
	mov al,cs:inum		;interrupt number vector we are changing
	mov ah,25h		;set interrupt vector dos function
	mov dx,cs:iseg
	mov ds,dx		;get saved interrupt vector segment
	mov dx,cs:ioff		;get saved interrupt vector offset
	int 21h			;perform dos function call

	MOV INTFLG,0		;clear interrupt flag
	MOV EFLAG,0		;and error flag
	JMP IORET		;and return to caller


@pdma32$qini endp
;****************************************************
;* keep spirit of CPP alive with naming conventions *
;****************************************************
;PDMA32	ENDP

	PAGE
;------ DELAY: SHORT DELAY ROUTINE --------------------------------------------

	COMMENT \

This routine introduces a short delay, required between back to back I/O
instructions on PC/AT.

e.c.\

  DELAY PROC NEAR
	NOP
	RET

  DELAY ENDP



	PAGE

;-------INTH: MODE 7 INTERRUPT SERVICE ROUTINE -------------------------------

	COMMENT \

This is an example dummy interrupt service routine that is installed by mode 7.
After stacking all the registers that are used and therefore altered by the
routine, you can add your own code to do whatever you wish. The exit section
clears the PDMA-32 interrupt by reading the PDMA-32 interrupt status register
restores the registers, and issues an EOI to the appropriate 8259 interrupt
controller which enables it to respond to an interrupt again (see Intel 8259
data sheet). If this routine in its present form is called by an interrupt,
itÿwill simply return without any action. 

The STI instruction re-enables the processor to respond to all interrupts, and
should normally be executed as soon after entry to the routine as possible
unless there is a good reason to delay. After the STI, higher priority
interrupts will be free to interrupt this interrupt routine. If you write a
long routine, other hardware peripherals e.g. COM ports may need this
precaution to operate correctly. Both the STI and EOI must always be issued
before exiting from the service routine. Mode 7 looks after installation of
the correct interrupt vectors for this routine, and enabling of the 8259
interrupt controller(s) and PDMA-32 on the correct level. Mode 8 terminates
interrupts.

To develop your own routine, use any text editor to expand and modify the code
below, re-assemble with the Macro Assembler and follow the instructions in
the file HOWTOBIN.DOC to generate a BASIC callable .BIN file.  

e.c.\

INTH PROC FAR

	push	ax		;save all registers
	push	bx		;since we are a "glue" routine, we must
	push	cx		;assume worst case register usage
	push	dx
	push	es
	push	ds
	push	si
	push	di
	push	bp

	STI			;re-enable processor interrupts

	MOV DX,CS:BASE		;get PDMA-32 base I/O address
	ADD DX,10		;address interrupt status register
	IN AL,DX		;read status and clear PDMA-32 interrupt
	DEC DX			;address interrupt level select register
	CALL DELAY		;PC/AT I/O delay
	IN AL,DX		;fetch selected interrupt level
	AND AX,000FH		;mask out data
	CMP AX,8		;is it handled by controller 1 or 2?
	JL ENC1			;re-enable controller 1
	MOV AL,20H		;or re-enable controller 2
	OUT 0A0H,AL		;issue EOI to controller 2
	CALL DELAY		;PC/AT I/O delay

ENC1: 	MOV AL,20H		;EOI code
	OUT 20H,AL		;issue EOI to controller 1
	call DELAY		;pc/at i/o delay

;Insert your own code here
;*********************************************************
;* to make life pleasant, we'll call an external routine *
;*********************************************************
	mov	ds,cs:BASIC	;restore BASIC's data segment
	call	far ptr @pdma32_interrupt$qv

	pop	bp		;restore all registers that were saved on
	pop	di		;the stack
	pop	si
	pop	ds
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax

	IRET			;return from interrupt

INTH ENDP

CSEG	ENDS			;end of segment

	END			;end of program

