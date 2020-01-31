; *************************
; * System Level Routines *
; *************************

	NAME	SYSTEM

AT_CHR	equ	1000000000000000b
AT_NUL	equ	0000000000000100b

; BIOS interrupts
EQUIP_CK	equ	11h

; DOS interrupts
BDOS_REQ	equ	21h

; DOS functions
GET_DATE	equ	2Ah
GET_TIME	equ	2Ch

	DOSSEG
	.MODEL	LARGE

EQUIP_STRUCT	STRUC
	par	dw	?
	ser	dw	?
	drives	dw	?
EQUIP_STRUCT	ENDS

	.DATA

	public	_equip

nuldev	db	'NUL     '
nulattr	dw	AT_CHR OR AT_NUL

devhead	STRUC
	next	dd	?
	attrib	dw	?
	strat	dw	?
	intrp	dw	?
	dname	db	8 dup (?)
	term	db	?
devhead	ENDS

_equip	EQUIP_STRUCT	<>

	.CODE

	public	@EquipInit$qv
	public	@ShortHi$ql
	public	@ShortLo$ql
	public	@ShortShort$qss
	public	@SysDate$qv
	public	@SysTime$qv

; ===========================================================================

; FUNCTION:	GetNumDevices
; DESCRIPTION:	Returns the number of block devices found.
; ERRORS:	None.

GetNumDevices	proc
	push	bp
	mov	bp,sp
	sub	sp,2
	push	si
	push	di
 	mov	ax,DGROUP		; set up data segment
 	mov	ds,ax
	mov	word ptr [bp-2],0	; clear total count
 	cld	
 	mov	cx,0FFFEh		; search count
 	xor	ax,ax			; start after INT table
 	mov	es,ax
 	mov	di,0400h
 	mov	al,[nuldev]		; beginning search value
search:
 	repne	scasb			; search until match
 	jne	exit			; no match
 	push	cx			; possible match ...
 	push	di			; ...save search position
 	mov	si,offset nuldev+1	; remainder "NUL" string
 	mov	cx,7			; remainder of count
 	repe	cmpsb			; compare until no match
 	jne	not_it			; didn't match
 	sub	di,(offset term - offset attrib) ; adjust pointer
 	cmpsw				; NUL's attribute word?
 	jne	not_it			; didn't match
 	add	sp,4			; discard saved DI & CX
 	sub	di,(offset strat - offset next) ; adjust pointer
 	jmp	short found_nul		; found NUL device header!
not_it:
 	pop	di			; restore position & count
 	pop	cx
 	jmp	search
found_nul:
 	test	es:[di+attrib],AT_CHR	; device char or block?
 	jnz	skip
 	xor	ah,ah
 	mov	al,es:[di+dname]	; number of units
	add	word ptr [bp-2],ax	; add it to our total count
skip:
 	cmp	word ptr es:[di],-1	; check for end of chain
 	je	exit			; if -1 then exit
 	les	di,es:[di+next]		; if not -1 then get next
 	jmp	found_nul		; continue
exit:
	mov	ax,word ptr [bp-2]	; prepare to return total count
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
GetNumDevices	endp

; ===========================================================================

; FUNCTION:	EquipInit
; DESCRIPTION:	Fills in equipment data structure.
; C++ SYNTAX:	void EquipInit(void);
; ERRORS:	None.
; COMMENTS:	Returned equipment format from BIOS call:
;		_______________  _______________
;		F E D C B A 9 8  7 6 5 4 3 2 1 0
;		X X . . . . . .  . . . . . . . .  number of printers
;		. . . X . . . .  . . . . . . . .  game adapters: 1 = installed
;		. . . . X X X .  . . . . . . . .  number of RS232 ports
;		. . . . . . . X  . . . . . . . .  DMA chip, 0=installed
;		. . . . . . . .  X X . . . . . .  x+1 = number of drives
;		. . . . . . . .  . . X X . . . .  initial video mode
;		. . . . . . . .  . . . . X X . .  system board RAM: 11 = 64K
;		. . . . . . . .  . . . . . . X .  1 if math co-processor installed
;		. . . . . . . .  . . . . . . . X  1 if any disk drives

@EquipInit$qv	proc
	push	bp
	mov	bp,sp
	push	si

	int	EQUIP_CK		; call BIOS for equipment list
	mov	si,ax			; save for later
	mov	cl,14			; parallel = (ax >> 14) & 0x03
	sar	ax,cl
	and	ax,3
	mov	[_equip.par],ax
	mov	ax,si			; serial = (ax >> 9) & 0x07
	mov	cl,9
	sar	ax,cl
	and	ax,7
	mov	[_equip.ser],ax
	call	GetNumDevices
	mov	[_equip.drives],ax

	pop	si
	pop	bp
	ret	
@EquipInit$qv	endp


; FUNCTION:	ShortHi
; DESCRIPTION:	Returns the high-order 16 bits of the 32 bit long val.
; C++ SYNTAX:	short ShortHi(long val);
; ERRORS:	None.

@ShortHi$ql	proc
	ARG	LO_ARG:WORD, HI_ARG:WORD	; let assemler worry about parameters
	push	bp
	mov	bp,sp
	mov	ax,[HI_ARG]			; pretend long passed as two shorts
	pop	bp
	ret	
@ShortHi$ql	endp


; FUNCTION:	ShortLo
; DESCRIPTION:	Returns the low-order 16 bits of the 32 bit long val.
; C++ SYNTAX:	short ShortLo(long val);
; ERRORS:	None.

@ShortLo$ql	proc
	ARG	LO_ARG:WORD, HI_ARG:WORD	; let assemler worry about parameters
	push	bp
	mov	bp,sp
	mov	ax,[LO_ARG]			; pretend long passed as two shorts
	pop	bp
	ret	
@ShortLo$ql	endp


; FUNCTION:	ShortShort
; DESCRIPTION:	Makes a long out of two shorts by putting high and low in the
;		high and low portions of the long return value respectively.
; C++ SYNTAX:	long ShortShort(short hi, short lo);
; ERRORS:	None.

@ShortShort$qss	proc
	ARG	HI_ARG:WORD, LO_ARG:WORD	; let assemler worry about parameters
	push	bp
	mov	bp,sp
	mov	dx,[HI_ARG]			; high
	mov	ax,[LO_ARG]			; low
	pop	bp
	ret	
@ShortShort$qss	endp


; FUNCTION:	SysDate
; DESCRIPTION:	Gets the system's date, convert it to DOS directory format.
; C++ SYNTAX:	Word SysDate(void);
; ERRORS:	No errors - system always has a date.
; COMMENTS:	DOS disk directory DATE format:
;		___________________  _______  _________
;		15 14 13 12 11 10 9  8 7 6 5  4 3 2 1 0
;			Year          Month      Day

@SysDate$qv	proc
	mov	ah,GET_DATE		; DOS function 2Ah
	int	BDOS_REQ		; call PC DOS
	mov	ax,cx			; get year
	sub	ax,1980			; get rid of DOS format year bias
	mov	cl,4			; shift count to make...
	shl	ax,cl			; ...room for month
	add	al,dh			; drop in month
	mov	cl,5			; shift count to make...
	shl	ax,cl			; ...room for day
	add	al,dl			; drop in day
	ret
@SysDate$qv	endp


; FUNCTION:	SysTime
; DESCRIPTION:	Gets the system's time, converts it to DOS directory format.
; C++ SYNTAX:	Word SysTime(void);
; ERRORS:	No errors - system always has a time.
; COMMENTS:	DOS disk directory TIME format:
;		______________  ____________  _________
;		15 14 13 12 11  10 9 8 7 6 5  4 3 2 1 0
;		     Hours        Minutes      Seconds

@SysTime$qv	proc
	mov	ah,GET_TIME		; DOS function 2Ch
	int	BDOS_REQ
	mov	ax,0			; clear AX (do minutes first to free CL)
	mov	al,cl			; get minutes
	mov	cl,5			; shift AX over 5 places...
	shl	ax,cl			; ...to make room for seconds
	mov	cl,3
	shl	ch,cl			; left justify hours
	add	ah,ch			; drop in hours
	shr	dh,1			; divide secs by two for 2-sec intervals
	add	al,dh
	ret
@SysTime$qv	endp

	end
