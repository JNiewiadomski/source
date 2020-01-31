; *************************
; * System Level Routines *
; *************************

	NAME	SYSTEM

; DOS interrupts
BDOS_REQ	equ	21h

; DOS functions
GET_DATE	equ	2Ah
GET_TIME	equ	2Ch

	DOSSEG
	.MODEL	LARGE

	.CODE

	public	_SysDate
	public	_SysTime

; FUNCTION:	SysDate
; DESCRIPTION:	Gets the system's date, convert it to DOS directory format.
; C++ SYNTAX:	Word SysDate(void);
; ERRORS:	No errors - system always has a date.
; COMMENTS:	DOS disk directory DATE format:
;		___________________  _______  _________
;		15 14 13 12 11 10 9  8 7 6 5  4 3 2 1 0
;			Year          Month      Day

_SysDate	proc
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
_SysDate	endp


; FUNCTION:	SysTime
; DESCRIPTION:	Gets the system's time, converts it to DOS directory format.
; C++ SYNTAX:	Word SysTime(void);
; ERRORS:	No errors - system always has a time.
; COMMENTS:	DOS disk directory TIME format:
;		______________  ____________  _________
;		15 14 13 12 11  10 9 8 7 6 5  4 3 2 1 0
;		     Hours        Minutes      Seconds

_SysTime	proc
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
_SysTime	endp

	end