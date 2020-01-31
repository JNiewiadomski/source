; ***************************
; * Critical Error Routines *
; ***************************

; DOS interrupts
BDOS_REQ	equ	21h

; DOS functions
SET_IVEC	equ	25h

; DOS Critical Error Handler Interrupt Vector Index
CRIT_ERR	equ	24h

	.MODEL	LARGE

	extrn	@CriticalErrorHandler$qus

	.DATA

	public	_errFlag
	public	_errInfo

_errFlag	db	0
_errInfo	dw	0

terminate	label	dword		; address of C terminate routine
	dw	0
	dw	0

	.CODE

	public	@CriticalInstall$qnqv$v


; FUNCTION:      CriticalHandler
; DESCRIPTION:   The critical error handler activated by interrupt CRIT_ERR

CriticalHandler proc
	sti				; turn interrupts back on
	push	ax			; save registers
	pushf
	push	bx			; save registers for writing messages
	push	cx
	push	dx
	push	ds
	push	es

	mov	ds,cs:critvect
	mov	word ptr DGROUP:_errInfo,ax

	mov	ax,di
	push	ax
	call	far ptr @CriticalErrorHandler$qus
	pop	cx

	pop	es			; done with messages: restore registers
	pop	ds
	pop	dx
	pop	cx
	pop	bx

	cmp	al,'f'			; branch on response value
	je	c03
	cmp	al,'F'
	je	c03
	cmp	al,'r'
	je	c04
	cmp	al,'R'
	je	c04
	cmp	al,'i'
	je	c05
	cmp	al,'I'
	je	c05

c02:	mov	byte ptr DGROUP:_errFlag,1
	popf				; abort
	pop	ax
	mov	ds,cs:critvect		; restore C data segment value
	call	dword ptr DGROUP:terminate  ; call C terminate function
	mov	ax,4C01h		; exit w/errorlevel 1
	int	BDOS_REQ		; call DOS

c03:	popf				; fail
	pop	ax
	mov	al,3			; 3 => fail
	iret				; back to DOS

c04:	popf				; retry
	pop	ax
	mov	al,1			; 1 => retry
	iret				; back to DOS

c05:	popf				; ignore
	pop	ax
	mov	al,0			; 0 => ignore
	iret				; back to DOS

critvect	dw	?		; store C DS value where addressible
					; by 'CriticalHandler'
CriticalHandler endp


; FUNCTION:      CriticalInstall
; DESCRIPTION:   Installs a critical error handler
; SYNTAX FROM C: void CriticalInstall(void (*terminateFunc)());
; ERRORS:        None.

@CriticalInstall$qnqv$v proc
	push	bp			; standard module initialization
	mov	bp,sp
	push	di
	push	si
	les	bx,dword ptr [bp+6]
	mov	word ptr DGROUP:terminate+2,es
	mov	word ptr DGROUP:terminate,bx
	mov	cs:critvect,ds		; save the C data segment for 'CriticalHandler'
	push	ds			; initialize interrupt CRIT_ERR to point
	mov	ah,SET_IVEC		; to 'CriticalHandler'
	push	cs
	pop	ds
	mov	dx,offset CriticalHandler
	mov	al,CRIT_ERR
	int	BDOS_REQ		; call DOS
	pop	ds
	pop	si			; restore registers
	pop	di
	mov	sp,bp
	pop	bp
	ret				; return to C program
@CriticalInstall$qnqv$v endp

	end
