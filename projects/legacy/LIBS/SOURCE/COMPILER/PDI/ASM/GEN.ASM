	ifndef	??version
?debug	macro
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	S "gen.cpp"
	?debug	C E92B5317150767656E2E637070
	?debug	C E90008A41417633A5C645C74635C696E636C7564655C616C6C6F63+
	?debug	C 2E68
	?debug	C E90008A41415633A5C645C74635C696E636C7564655C646F732E68
	?debug	C E90008A41418633A5C645C74635C696E636C7564655C7374646172+
	?debug	C 672E68
	?debug	C E90008A41417633A5C645C74635C696E636C7564655C737464696F+
	?debug	C 2E68
	?debug	C E90008A41418633A5C645C74635C696E636C7564655C7374646C69+
	?debug	C 622E68
	?debug	C E90008A41418633A5C645C74635C696E636C7564655C737472696E+
	?debug	C 672E68
	?debug	C E94B50FA141F633A5C645C74635C696E636C7564655C636F6D6D6F+
	?debug	C 6E5C636F6D6D6F6E2E68
	?debug	C E9567DB6141E633A5C645C74635C696E636C7564655C636F6D6D6F+
	?debug	C 6E5C6A696666792E68
	?debug	C E98281FB1421633A5C645C74635C696E636C7564655C636F6D6D6F+
	?debug	C 6E5C6B6579626F6172642E68
	?debug	C E9215EB714076572726F722E68
	?debug	C E92246BD140567656E2E68
	?debug	C E9F7531715056C65782E68
	?debug	C E94373B9140873796D626F6C2E68
	?debug	C E9E97EC4140A73656D616E7469632E68
	?debug	C E94A82B0141070646D612D33325C70646D6133322E68
GEN_TEXT	segment byte public 'CODE'
GEN_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:GEN_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_DATA	segment word public 'DATA'
KPressTab	label	word
	dd	DGROUP:s@
	db	80
	db	1
	dd	DGROUP:s@+10
	db	79
	db	1
	dd	DGROUP:s@+19
	db	195
	db	1
	dd	DGROUP:s@+30
	db	27
	db	0
	dd	DGROUP:s@+39
	db	68
	db	1
	dd	DGROUP:s@+48
	db	59
	db	1
	dd	DGROUP:s@+56
	db	60
	db	1
	dd	DGROUP:s@+64
	db	61
	db	1
	dd	DGROUP:s@+72
	db	62
	db	1
	dd	DGROUP:s@+80
	db	63
	db	1
	dd	DGROUP:s@+88
	db	64
	db	1
	dd	DGROUP:s@+96
	db	65
	db	1
	dd	DGROUP:s@+104
	db	66
	db	1
	dd	DGROUP:s@+112
	db	67
	db	1
	dd	DGROUP:s@+120
	db	71
	db	1
	dd	DGROUP:s@+130
	db	75
	db	1
	dd	DGROUP:s@+140
	db	81
	db	1
	dd	DGROUP:s@+150
	db	73
	db	1
	dd	DGROUP:s@+160
	db	77
	db	1
	dd	DGROUP:s@+171
	db	32
	db	0
	dd	DGROUP:s@+182
	db	193
	db	1
	dd	DGROUP:s@+191
	db	72
	db	1
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
pageTable	label	word
	db	0
	db	0
	db	0
	db	16
	db	0
	db	32
	db	0
	db	48
	db	0
	db	64
	db	0
	db	80
	db	0
	db	96
	db	0
	db	112
	db	0
	db	128
	db	0
	db	144
	db	0
	db	160
	db	0
	db	176
	db	0
	db	192
	db	0
	db	208
	db	0
	db	224
	db	0
	db	240
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseState	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseStateLast	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseKey	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseKeyLast	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseStartup	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
caseStartupLast	label	dword
	db	0
	db	0
	db	0
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_DATA	segment word public 'DATA'
stateChange	label	word
	db	1
	db	0
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
GEN_TEXT	segment byte public 'CODE'
   ;	
   ;	void GenCaseAction(void *obj, char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseAction$qnvnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,12
   ;	
   ;		Symbol		*sym;
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((sym = SymbolFind(id)) == NULL) {
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @1@98
   ;	
   ;			ErrorSevere("Referencing undefined .ACTION '%s'", id);
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ds
	mov	ax,offset DGROUP:s@+199
@1@74:
	push	ax
	call	far ptr @ErrorSevere$qnzce
	add	sp,8
   ;	
   ;			return;
   ;	
	jmp	@1@482
@1@98:
   ;	
   ;			}
   ;		if (!IS_ACTION(sym)) {
   ;	
	les	bx,dword ptr [bp-4]
	cmp	word ptr es:[bx+33],0
	je	short @1@146
   ;	
   ;			ErrorSevere("'%s' is not of type .ACTION", id);
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ds
	mov	ax,offset DGROUP:s@+234
	jmp	short @1@74
@1@146:
   ;	
   ;			return;
   ;			}
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-8],ax
	mov	word ptr [bp-6],dx
	or	ax,dx
	jne	short @1@194
   ;	
   ;			e_OutOfMemory("GenCaseActionState1");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+262
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@1@194:
   ;	
   ;		action->type = A_ACTION;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx],0
   ;	
   ;		if ((action->u.id = strdup(id)) == NULL)
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _strdup
	pop	cx
	pop	cx
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+2],ax
	mov	word ptr es:[bx+4],dx
	or	ax,dx
	jne	short @1@242
   ;	
   ;			e_OutOfMemory("GenCaseActionState2");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+282
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@1@242:
   ;	
   ;		action->next = NULL;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @1@362
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @1@314
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @1@338
@1@314:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@1@338:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @1@482
@1@362:
   ;	
   ;			}
   ;		else {			/* case list */
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-12],dx
	mov	word ptr [bp-10],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @1@410
   ;	
   ;				state->action = action;
   ;	
	jmp	short @1@434
@1@410:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-12]
	les	bx,dword ptr es:[bx+10]
@1@434:
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@1@482:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseAction$qnvnzc	endp
   ;	
   ;	void GenCaseActionBeep(void *obj, Word val) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseActionBeep$qnvus	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @2@74
   ;	
   ;			e_OutOfMemory("GenCaseActionBeep");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+302
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@2@74:
   ;	
   ;		action->type = A_BEEP;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx],1
   ;	
   ;		action->u.val = val;
   ;	
	mov	ax,word ptr [bp+10]
	mov	word ptr es:[bx+2],ax
   ;	
   ;		action->next = NULL;
   ;	
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @2@194
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @2@146
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @2@170
@2@146:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@2@170:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @2@314
@2@194:
   ;	
   ;			}
   ;		else {			/* case list */
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @2@242
   ;	
   ;				state->action = action;
   ;	
	jmp	short @2@266
@2@242:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-8]
	les	bx,dword ptr es:[bx+10]
@2@266:
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@2@314:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseActionBeep$qnvus	endp
   ;	
   ;	void GenCaseActionClear(void *obj, Word val) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseActionClear$qnvus	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @3@74
   ;	
   ;			e_OutOfMemory("GenCaseActionClear");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+320
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@3@74:
   ;	
   ;		action->type = A_CLEAR;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx],2
   ;	
   ;		action->u.val = val;
   ;	
	mov	ax,word ptr [bp+10]
	mov	word ptr es:[bx+2],ax
   ;	
   ;		action->next = NULL;
   ;	
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @3@194
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @3@146
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @3@170
@3@146:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@3@170:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @3@314
@3@194:
   ;	
   ;			}
   ;		else {			/* case list */
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @3@242
   ;	
   ;				state->action = action;
   ;	
	jmp	short @3@266
@3@242:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-8]
	les	bx,dword ptr es:[bx+10]
@3@266:
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@3@314:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseActionClear$qnvus	endp
   ;	
   ;	void GenCaseActionEcho(void *obj, char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseActionEcho$qnvnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,12
   ;	
   ;		Symbol		*sym;
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((sym = SymbolFind(id)) == NULL) {
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @4@98
   ;	
   ;			ErrorSevere("Referencing undefined .TEXT '%s'", id);
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ds
	mov	ax,offset DGROUP:s@+339
@4@74:
	push	ax
	call	far ptr @ErrorSevere$qnzce
	add	sp,8
   ;	
   ;			return;
   ;	
	jmp	@4@482
@4@98:
   ;	
   ;			}
   ;		if (!IS_TEXT(sym)) {
   ;	
	les	bx,dword ptr [bp-4]
	cmp	word ptr es:[bx+33],2
	je	short @4@146
   ;	
   ;			ErrorSevere("'%s' is not of type .TEXT", id);
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ds
	mov	ax,offset DGROUP:s@+372
	jmp	short @4@74
@4@146:
   ;	
   ;			return;
   ;			}
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-8],ax
	mov	word ptr [bp-6],dx
	or	ax,dx
	jne	short @4@194
   ;	
   ;			e_OutOfMemory("GenCaseActionEcho1");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+398
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@4@194:
   ;	
   ;		action->type = A_ECHO;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx],3
   ;	
   ;		if ((action->u.id = strdup(id)) == NULL)
   ;	
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _strdup
	pop	cx
	pop	cx
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+2],ax
	mov	word ptr es:[bx+4],dx
	or	ax,dx
	jne	short @4@242
   ;	
   ;			e_OutOfMemory("GenCaseActionEcho2");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+417
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@4@242:
   ;	
   ;		action->next = NULL;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @4@362
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @4@314
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @4@338
@4@314:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@4@338:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @4@482
@4@362:
   ;	
   ;			}
   ;		else {			/* case list */
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-12],dx
	mov	word ptr [bp-10],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @4@410
   ;	
   ;				state->action = action;
   ;	
	jmp	short @4@434
@4@410:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-12]
	les	bx,dword ptr es:[bx+10]
@4@434:
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@4@482:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseActionEcho$qnvnzc	endp
   ;	
   ;	void GenCaseActionHalt(void *obj) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseActionHalt$qnv	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @5@74
   ;	
   ;			e_OutOfMemory("GenCaseActionHalt");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+436
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@5@74:
   ;	
   ;		action->type = A_HALT;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx],4
   ;	
   ;		action->next = NULL;
   ;	
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @5@194
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @5@146
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @5@170
@5@146:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@5@170:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @5@314
@5@194:
   ;	
   ;			}
   ;		else {
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @5@242
   ;	
   ;				state->action = action;
   ;	
	jmp	short @5@266
@5@242:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-8]
	les	bx,dword ptr es:[bx+10]
@5@266:
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@5@314:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseActionHalt$qnv	endp
   ;	
   ;	void GenCaseActionSet(void *obj, Word val) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseActionSet$qnvus	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		case_action	*action;
   ;		case_state	*state;
   ;	
   ;		if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
   ;	
	mov	ax,10
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @6@74
   ;	
   ;			e_OutOfMemory("GenCaseActionSet");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+454
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@6@74:
   ;	
   ;		action->type = A_SET;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx],5
   ;	
   ;		action->u.val = val;
   ;	
	mov	ax,word ptr [bp+10]
	mov	word ptr es:[bx+2],ax
   ;	
   ;		action->next = NULL;
   ;	
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		if (obj == NULL) {	/* startup list */
   ;	
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	short @6@194
   ;	
   ;			if (caseStartup == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseStartup
	or	ax,word ptr DGROUP:caseStartup+2
	jne	short @6@146
   ;	
   ;				caseStartup = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartup,dx
	mov	word ptr DGROUP:caseStartup+2,ax
	jmp	short @6@170
@6@146:
   ;	
   ;			else
   ;				caseStartupLast->next = action;
   ;	
	les	bx,dword ptr DGROUP:caseStartupLast
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
@6@170:
   ;	
   ;			caseStartupLast = action;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseStartupLast,dx
	mov	word ptr DGROUP:caseStartupLast+2,ax
	jmp	short @6@314
@6@194:
   ;	
   ;			}
   ;		else {			/* case list */
   ;			state = (case_state *)obj;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
   ;	
   ;			if (state->action == NULL)
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+6]
	or	ax,word ptr es:[bx+8]
	jne	short @6@242
   ;	
   ;				state->action = action;
   ;	
	jmp	short @6@266
@6@242:
   ;	
   ;			else
   ;				state->actionLast->next = action;
   ;	
	les	bx,dword ptr [bp-8]
	les	bx,dword ptr es:[bx+10]
@6@266:
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+6],dx
	mov	word ptr es:[bx+8],ax
   ;	
   ;			state->actionLast = action;
   ;	
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+10],dx
	mov	word ptr es:[bx+12],ax
@6@314:
   ;	
   ;			}
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseActionSet$qnvus	endp
   ;	
   ;	void *GenCaseKey(char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseKey$qnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
   ;	
   ;		case_state	*key;
   ;	
   ;		if ((key = (case_state *) malloc(sizeof(case_state))) == NULL)
   ;	
	mov	ax,18
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @7@74
   ;	
   ;			e_OutOfMemory("GenCaseKey1");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+471
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@7@74:
   ;	
   ;		key->type = C_KEY;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx],0
   ;	
   ;		if ((key->u.ch = GenCheckKeypress(id)) == K_NULL)
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr @GenCheckKeypress$qnzc
	pop	cx
	pop	cx
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx+2],ax
	cmp	ax,259
	jne	short @7@122
   ;	
   ;			e_Internal("GenCaseKey2");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+483
	push	ax
	mov	ax,1
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@7@122:
   ;	
   ;		key->action = NULL;
   ;	
	les	bx,dword ptr [bp-4]
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		key->actionLast = NULL;
   ;	
	mov	word ptr es:[bx+10],0
	mov	word ptr es:[bx+12],0
   ;	
   ;		key->next = NULL;
   ;	
	mov	word ptr es:[bx+14],0
	mov	word ptr es:[bx+16],0
   ;	
   ;		if (caseKey == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseKey
	or	ax,word ptr DGROUP:caseKey+2
	jne	short @7@170
   ;	
   ;			caseKey = key;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseKey,dx
	mov	word ptr DGROUP:caseKey+2,ax
	jmp	short @7@194
@7@170:
   ;	
   ;		else
   ;			caseKeyLast->next = key;
   ;	
	les	bx,dword ptr DGROUP:caseKeyLast
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr es:[bx+14],dx
	mov	word ptr es:[bx+16],ax
@7@194:
   ;	
   ;		caseKeyLast = key;
   ;	
	mov	ax,word ptr [bp-2]
	mov	dx,word ptr [bp-4]
	mov	word ptr DGROUP:caseKeyLast,dx
	mov	word ptr DGROUP:caseKeyLast+2,ax
   ;	
   ;		return (key);
   ;	
	mov	dx,word ptr [bp-2]
	mov	ax,word ptr [bp-4]
   ;	
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseKey$qnzc	endp
   ;	
   ;	void *GenCaseState(char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenCaseState$qnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		Symbol		*sym;
   ;		case_state	*state;
   ;	
   ;		if ((sym = SymbolFind(id)) == NULL) {
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	or	ax,dx
	jne	short @8@98
   ;	
   ;			ErrorSevere("Referencing undefined .STATE '%s'", id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+495
@8@74:
	push	ax
	call	far ptr @ErrorSevere$qnzce
	add	sp,8
   ;	
   ;			return (NULL);
   ;	
	xor	dx,dx
	xor	ax,ax
	jmp	@8@338
@8@98:
   ;	
   ;			}
   ;		if (!IS_STATE(sym)) {
   ;	
	les	bx,dword ptr [bp-4]
	cmp	word ptr es:[bx+33],1
	je	short @8@146
   ;	
   ;			ErrorSevere("'%s' is not of type .STATE", id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+529
	jmp	short @8@74
@8@146:
   ;	
   ;			return (NULL);
   ;			}
   ;		if ((state = (case_state *) malloc(sizeof(case_state))) == NULL)
   ;	
	mov	ax,18
	push	ax
	call	far ptr _malloc
	pop	cx
	mov	word ptr [bp-8],ax
	mov	word ptr [bp-6],dx
	or	ax,dx
	jne	short @8@194
   ;	
   ;			e_OutOfMemory("GenCaseState1");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+556
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@8@194:
   ;	
   ;		state->type = C_STATE;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx],1
   ;	
   ;		if ((state->u.id = strdup(id)) == NULL)
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _strdup
	pop	cx
	pop	cx
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+2],ax
	mov	word ptr es:[bx+4],dx
	or	ax,dx
	jne	short @8@242
   ;	
   ;			e_OutOfMemory("GenCaseState2");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+570
	push	ax
	xor	ax,ax
	push	ax
	call	far ptr @ErrorFatalList$qsnzc
	add	sp,6
@8@242:
   ;	
   ;		state->action = NULL;
   ;	
	les	bx,dword ptr [bp-8]
	mov	word ptr es:[bx+6],0
	mov	word ptr es:[bx+8],0
   ;	
   ;		state->actionLast = NULL;
   ;	
	mov	word ptr es:[bx+10],0
	mov	word ptr es:[bx+12],0
   ;	
   ;		state->next = NULL;
   ;	
	mov	word ptr es:[bx+14],0
	mov	word ptr es:[bx+16],0
   ;	
   ;		if (caseState == NULL)
   ;	
	mov	ax,word ptr DGROUP:caseState
	or	ax,word ptr DGROUP:caseState+2
	jne	short @8@290
   ;	
   ;			caseState = state;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseState,dx
	mov	word ptr DGROUP:caseState+2,ax
	jmp	short @8@314
@8@290:
   ;	
   ;		else
   ;			caseStateLast->next = state;
   ;	
	les	bx,dword ptr DGROUP:caseStateLast
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr es:[bx+14],dx
	mov	word ptr es:[bx+16],ax
@8@314:
   ;	
   ;		caseStateLast = state;
   ;	
	mov	ax,word ptr [bp-6]
	mov	dx,word ptr [bp-8]
	mov	word ptr DGROUP:caseStateLast,dx
	mov	word ptr DGROUP:caseStateLast+2,ax
   ;	
   ;		return (state);
   ;	
	mov	dx,word ptr [bp-6]
	mov	ax,word ptr [bp-8]
@8@338:
   ;	
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCaseState$qnzc	endp
   ;	
   ;	static short GenCheckKeypress(char *name) {
   ;	
	assume	cs:GEN_TEXT
@GenCheckKeypress$qnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,10
   ;	
   ;		KPRESS	*found, dummy;
   ;	
   ;		dummy.name = name;
   ;	
	mov	ax,word ptr [bp+8]
	mov	dx,word ptr [bp+6]
	mov	word ptr [bp-10],dx
	mov	word ptr [bp-8],ax
   ;	
   ;		found = (KPRESS *) bsearch(&dummy, KPressTab, sizeof(KPressTab)/sizeof(KPRESS), sizeof(KPRESS), GenCompare);
   ;	
	mov	ax,seg @GenCompare$qnxvt1
	push	ax
	mov	ax,offset @GenCompare$qnxvt1
	push	ax
	mov	ax,6
	push	ax
	mov	ax,22
	push	ax
	push	ds
	mov	ax,offset DGROUP:KPressTab
	push	ax
	push	ss
	lea	ax,word ptr [bp-10]
	push	ax
	call	far ptr _bsearch
	add	sp,16
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
   ;	
   ;		return (found ? found->key : K_NULL);
   ;	
	mov	ax,word ptr [bp-4]
	or	ax,dx
	je	short @9@74
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+4]
	jmp	short @9@98
@9@74:
	mov	ax,259
@9@98:
   ;	
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenCheckKeypress$qnzc	endp
   ;	
   ;	static int GenCompare(const void *a, const void *b) {
   ;	
	assume	cs:GEN_TEXT
@GenCompare$qnxvt1	proc	far
	push	bp
	mov	bp,sp
   ;	
   ;		return (strcmp(((KPRESS *)a)->name, ((KPRESS *)b)->name));
   ;	
	les	bx,dword ptr [bp+10]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	call	far ptr _strcmp
	add	sp,8
   ;	
   ;		}
   ;	
	pop	bp
	ret	
@GenCompare$qnxvt1	endp
   ;	
   ;	void GenStart(void) {
   ;	
	assume	cs:GEN_TEXT
@GenStart$qv	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
	push	si
   ;	
   ;		char	huge *block;
   ;		DWord	size;
   ;		short	i;
   ;	
   ;		md = PDMA_INITIALIZE;
   ;	
	mov	word ptr DGROUP:md,0
   ;	
   ;		d[0] = dev.baseAddress;
   ;	
	mov	ax,word ptr DGROUP:_dev+10
	mov	word ptr DGROUP:d,ax
   ;	
   ;		d[1] = dev.dmaLevel;
   ;	
	mov	ax,word ptr DGROUP:_dev+12
	mov	word ptr DGROUP:d+2,ax
   ;	
   ;		d[2] = dev.interruptLevel;
   ;	
	mov	ax,word ptr DGROUP:_dev+14
	mov	word ptr DGROUP:d+4,ax
   ;	
   ;		d[3] = dev.portMode;
   ;	
	mov	ax,word ptr DGROUP:_dev+16
	mov	word ptr DGROUP:d+6,ax
   ;	
   ;		if (pdma32(md, d) != 0)
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
	or	ax,ax
	je	short @11@74
   ;	
   ;			ErrorRunTime("PDMA-32 hardware not found");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+584
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	pop	cx
	pop	cx
@11@74:
   ;	
   ;		size = dev.portMode==DEV_BYTE ? BYTE_BLOCK : WORD_BLOCK;
   ;	
	cmp	word ptr DGROUP:_dev+16,0
	jne	short @11@122
	mov	dx,2
	xor	ax,ax
	jmp	short @11@146
@11@122:
	mov	dx,3
	xor	ax,ax
@11@146:
	mov	word ptr [bp-8],ax
	mov	word ptr [bp-6],dx
   ;	
   ;		if ((block = (char huge *)farmalloc(size)) == NULL)
   ;	
	push	dx
	push	word ptr [bp-8]
	call	far ptr _farmalloc
	pop	cx
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
	xor	cx,cx
	xor	bx,bx
	call	far ptr F_PCMP@
	jne	short @11@194
   ;	
   ;			ErrorRunTime("Unable to allocate sufficient far memory");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+611
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	pop	cx
	pop	cx
@11@194:
   ;	
   ;		dev.buffSeg = FP_SEG(block);
   ;	
	mov	ax,word ptr [bp-2]
	mov	word ptr DGROUP:_dev+26,ax
   ;	
   ;		for (i=0; dev.buffSeg >= pageTable[i]; i++)
   ;	
	xor	si,si
	jmp	short @11@242
@11@218:
	inc	si
@11@242:
	mov	bx,si
	shl	bx,1
	mov	ax,word ptr DGROUP:pageTable[bx]
	cmp	ax,word ptr DGROUP:_dev+26
	jbe	short @11@218
   ;	
   ;			;
   ;		dev.buffSeg = pageTable[i];
   ;	
	mov	bx,si
	shl	bx,1
	mov	ax,word ptr DGROUP:pageTable[bx]
	mov	word ptr DGROUP:_dev+26,ax
   ;	
   ;		dev.buffOff = 0;
   ;	
	mov	word ptr DGROUP:_dev+28,0
   ;	
   ;		md = PDMA_SET_TIMER;
   ;	
	mov	word ptr DGROUP:md,3
   ;	
   ;		d[0] = dev.counter0;
   ;	
	mov	ax,word ptr DGROUP:_dev+30
	mov	word ptr DGROUP:d,ax
   ;	
   ;		d[1] = dev.counter1;
   ;	
	mov	ax,word ptr DGROUP:_dev+32
	mov	word ptr DGROUP:d+2,ax
   ;	
   ;		if (pdma32(md, d) != 0)
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
	or	ax,ax
	je	short @11@314
   ;	
   ;			ErrorRunTime("Unable to set PDMA-32 timers");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+652
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	pop	cx
	pop	cx
@11@314:
   ;	
   ;		md = PDMA_AUX_OUTPUT;
   ;	
	mov	word ptr DGROUP:md,6
   ;	
   ;		for (i=NUMELE(dev.aux)-1; i>=0; i--)
   ;	
	mov	si,2
	jmp	short @11@386
@11@338:
   ;	
   ;			d[i] = 0;
   ;	
	mov	bx,si
	shl	bx,1
	mov	word ptr DGROUP:d[bx],0
	dec	si
@11@386:
	or	si,si
	jge	short @11@338
   ;	
   ;		if (pdma32(md, d) != 0)
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
	or	ax,ax
	je	short @11@458
   ;	
   ;			ErrorRunTime("Unable to clear auxiliary outputs");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+681
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	pop	cx
	pop	cx
@11@458:
   ;	
   ;		md = PDMA_INTERRUPT_ENABLE;
   ;	
	mov	word ptr DGROUP:md,7
   ;	
   ;		d[0] = dev.interruptSource;
   ;	
	mov	ax,word ptr DGROUP:_dev+50
	mov	word ptr DGROUP:d,ax
   ;	
   ;		if (pdma32(md, d) != 0)
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
	or	ax,ax
	je	short @11@506
   ;	
   ;			ErrorRunTime("Unable to enable hardware interrupt");
   ;	
	push	ds
	mov	ax,offset DGROUP:s@+715
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	pop	cx
	pop	cx
@11@506:
   ;	
   ;		if (dev.clockSource == DEV_CLOCK_TIMER)
   ;	
	cmp	word ptr DGROUP:_dev+24,1
	jne	short @11@554
   ;	
   ;			GenPrint("Clock rate set to %.15g Hz\n", dev.frequency);
   ;	
	FLD	qword ptr DGROUP:_dev+34
	add	sp,65528
	FSTP	qword ptr [bp-18]
	push	ds
	mov	ax,offset DGROUP:s@+751
	push	ax
	FWAIT	
	call	far ptr @GenPrint$qnzce
	add	sp,12
@11@554:
   ;	
   ;		if (GenPerform(caseStartup))
   ;	
	push	word ptr DGROUP:caseStartup+2
	push	word ptr DGROUP:caseStartup
	call	far ptr @GenPerform$qn11case_action
	pop	cx
	pop	cx
	or	ax,ax
	je	short @11@602
   ;	
   ;			GenStates();
   ;	
	call	far ptr @GenStates$qv
@11@602:
   ;	
   ;		GenEnd();
   ;	
	call	far ptr @GenEnd$qv
   ;	
   ;		}
   ;	
	pop	si
	mov	sp,bp
	pop	bp
	ret	
@GenStart$qv	endp
   ;	
   ;	static void GenEnd(void) {
   ;	
	assume	cs:GEN_TEXT
@GenEnd$qv	proc	far
	push	bp
	mov	bp,sp
   ;	
   ;		md = PDMA_INTERRUPT_DISABLE;	/* turn off ISR */
   ;	
	mov	word ptr DGROUP:md,8
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		md = PDMA_SETUP_TRANSFER;
   ;	
	mov	word ptr DGROUP:md,1
   ;	
   ;		d[0] = 1;			/* 1 bytes/words to transfer */
   ;	
	mov	word ptr DGROUP:d,1
   ;	
   ;		d[2] = DEV_INPUT;		/* data direction */
   ;	
	mov	word ptr DGROUP:d+4,0
   ;	
   ;		d[3] = DEV_AUTO_OFF;		/* auto-recycle off */
   ;	
	mov	word ptr DGROUP:d+6,0
   ;	
   ;		d[4] = DEV_CLOCK_TIMER;		/* transfer clock source */
   ;	
	mov	word ptr DGROUP:d+8,1
   ;	
   ;		d[5] = dev.buffSeg;
   ;	
	mov	ax,word ptr DGROUP:_dev+26
	mov	word ptr DGROUP:d+10,ax
   ;	
   ;		d[6] = dev.buffOff;
   ;	
	mov	ax,word ptr DGROUP:_dev+28
	mov	word ptr DGROUP:d+12,ax
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		md = PDMA_DIGITAL_OUTPUT;	/* turn off all outputs */
   ;	
	mov	word ptr DGROUP:md,4
   ;	
   ;		d[0] = 0;
   ;	
	mov	word ptr DGROUP:d,0
   ;	
   ;		d[1] = PDMA_PORTB;
   ;	
	mov	word ptr DGROUP:d+2,1
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		md = PDMA_AUX_OUTPUT;		/* turn off all auxiliary ports */
   ;	
	mov	word ptr DGROUP:md,6
   ;	
   ;		d[0] = d[1] = d[2] = 0;
   ;	
	xor	ax,ax
	mov	word ptr DGROUP:d+4,ax
	mov	word ptr DGROUP:d+2,ax
	mov	word ptr DGROUP:d,ax
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		}
   ;	
	pop	bp
	ret	
@GenEnd$qv	endp
   ;	
   ;	static void GenStates(void) {
   ;	
	assume	cs:GEN_TEXT
@GenStates$qv	proc	far
	push	bp
	mov	bp,sp
	sub	sp,12
	push	si
	push	di
   ;	
   ;		case_state	*state;
   ;		short		ch;
   ;		Word		data, cmpVal;
   ;		Link		*lk;
   ;		Boolean		done;
   ;	
   ;		if (caseKey==NULL && caseState==NULL)
   ;	
	mov	ax,word ptr DGROUP:caseKey
	or	ax,word ptr DGROUP:caseKey+2
	jne	short @13@74
	mov	ax,word ptr DGROUP:caseState
	or	ax,word ptr DGROUP:caseState+2
	jne	@@0
	jmp	@13@650
@@0:
@13@74:
   ;	
   ;			return;
   ;		while (true) {
   ;			if (KeyboardAvail() && (state = caseKey)!=NULL) {
   ;	
	call	far ptr @KeyboardAvail$qv
	or	ax,ax
	je	short @13@314
	mov	ax,word ptr DGROUP:caseKey+2
	mov	dx,word ptr DGROUP:caseKey
	mov	word ptr [bp-4],dx
	mov	word ptr [bp-2],ax
	or	dx,ax
	je	short @13@314
   ;	
   ;				for (done=false, ch=KeyboardRead(); !done && state!=NULL; state=state->next)
   ;	
	xor	si,si
	call	far ptr @KeyboardRead$qv
	mov	word ptr [bp-6],ax
	jmp	short @13@242
@13@146:
   ;	
   ;					if (state->u.ch == ch) {
   ;	
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+2]
	cmp	ax,word ptr [bp-6]
	jne	short @13@218
   ;	
   ;						if (!GenPerform(state->action))
   ;	
	push	word ptr es:[bx+8]
	push	word ptr es:[bx+6]
	call	far ptr @GenPerform$qn11case_action
	pop	cx
	pop	cx
	or	ax,ax
	jne	@@1
	jmp	@13@650
@@1:
   ;	
   ;							return;
   ;						done = true;
   ;	
	mov	si,1
@13@218:
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+16]
	mov	dx,word ptr es:[bx+14]
	mov	word ptr [bp-4],dx
	mov	word ptr [bp-2],ax
@13@242:
	or	si,si
	jne	short @13@74
	mov	ax,word ptr [bp-4]
	or	ax,word ptr [bp-2]
	jne	short @13@146
	jmp	short @13@74
@13@314:
   ;	
   ;						}
   ;				}
   ;			else if ((state = caseState) != NULL) {
   ;	
	mov	ax,word ptr DGROUP:caseState+2
	mov	dx,word ptr DGROUP:caseState
	mov	word ptr [bp-4],dx
	mov	word ptr [bp-2],ax
	or	dx,ax
	je	short @13@74
   ;	
   ;				md = PDMA_DIGITAL_INPUT;
   ;	
	mov	word ptr DGROUP:md,5
   ;	
   ;				d[1] = PDMA_PORTB;
   ;	
	mov	word ptr DGROUP:d+2,1
   ;	
   ;				if (dev.portMode==DEV_BYTE)
   ;	
	cmp	word ptr DGROUP:_dev+16,0
	jne	short @13@386
   ;	
   ;					d[2] = DEV_CONFIG_NO_CHANGE;
   ;	
	mov	word ptr DGROUP:d+4,0
	jmp	short @13@410
@13@386:
   ;	
   ;				else
   ;					d[2] = DEV_CONFIG_CHANGE;
   ;	
	mov	word ptr DGROUP:d+4,1
@13@410:
   ;	
   ;				pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;				data = d[0];
   ;	
	mov	ax,word ptr DGROUP:d
	mov	word ptr [bp-8],ax
   ;	
   ;				if (dev.portMode == DEV_WORD)
   ;	
	cmp	word ptr DGROUP:_dev+16,1
	jne	short @13@458
   ;	
   ;					data >>= BYTE_BIT;
   ;	
	mov	cl,8
	shr	word ptr [bp-8],cl
@13@458:
   ;	
   ;				for (done=false; !done && state!=NULL; state=state->next) {
   ;	
	xor	si,si
	jmp	short @13@578
@13@482:
   ;	
   ;					lk = SymbolFind(state->u.id)->val;
   ;	
	les	bx,dword ptr [bp-4]
	push	word ptr es:[bx+4]
	push	word ptr es:[bx+2]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	bx,ax
	mov	es,dx
	mov	ax,word ptr es:[bx+37]
	mov	dx,word ptr es:[bx+35]
	mov	word ptr [bp-12],dx
	mov	word ptr [bp-10],ax
   ;	
   ;					cmpVal = data & lk->u.state.andMask;
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr [bp-8]
	and	ax,word ptr es:[bx]
	mov	di,ax
   ;	
   ;					if (cmpVal == lk->u.state.cmpMask) {
   ;	
	cmp	word ptr es:[bx+2],ax
	jne	short @13@554
   ;	
   ;						if (!GenPerform(state->action))
   ;	
	les	bx,dword ptr [bp-4]
	push	word ptr es:[bx+8]
	push	word ptr es:[bx+6]
	call	far ptr @GenPerform$qn11case_action
	pop	cx
	pop	cx
	or	ax,ax
	je	short @13@650
   ;	
   ;							return;
   ;						done = true;
   ;	
	mov	si,1
@13@554:
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+16]
	mov	dx,word ptr es:[bx+14]
	mov	word ptr [bp-4],dx
	mov	word ptr [bp-2],ax
@13@578:
	or	si,si
	je	@@2
	jmp	@13@74
@@2:
	mov	ax,word ptr [bp-4]
	or	ax,word ptr [bp-2]
	jne	short @13@482
	jmp	@13@74
@13@650:
   ;	
   ;						}
   ;					}
   ;				}
   ;			}
   ;		}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
@GenStates$qv	endp
   ;	
   ;	static Boolean GenPerform(case_action *action) {
   ;	
	assume	cs:GEN_TEXT
@GenPerform$qn11case_action	proc	far
	push	bp
	mov	bp,sp
	jmp	@14@362
@14@50:
   ;	
   ;		for ( ; action!=NULL; action=action->next) {
   ;			switch (action->type) {
   ;	
	les	bx,dword ptr [bp+6]
	mov	bx,word ptr es:[bx]
	cmp	bx,5
	jbe	@@3
	jmp	@14@338
@@3:
	shl	bx,1
	jmp	word ptr cs:@14@C462[bx]
@14@122:
   ;	
   ;				case A_ACTION:
   ;					stateChange = false;
   ;	
	mov	word ptr DGROUP:stateChange,0
   ;	
   ;					GenPerformAction(action->u.id);
   ;	
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+4]
	push	word ptr es:[bx+2]
	call	far ptr @GenPerformAction$qnzc
	pop	cx
	pop	cx
@14@146:
   ;	
   ;					while (!stateChange)
   ;	
	cmp	word ptr DGROUP:stateChange,0
	je	short @14@146
	jmp	short @14@338
@14@194:
   ;	
   ;						;
   ;					break;
   ;				case A_BEEP:
   ;					sound(action->u.val);
   ;	
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	call	far ptr _sound
	pop	cx
   ;	
   ;					JiffyDelay(30, NULL);
   ;	
	xor	ax,ax
	xor	dx,dx
	push	ax
	push	dx
	mov	dx,30
	push	ax
	push	dx
	call	far ptr @JiffyDelay$qulnqv$v
	add	sp,8
   ;	
   ;					nosound();
   ;	
	call	far ptr _nosound
	jmp	short @14@338
@14@218:
   ;	
   ;					break;
   ;				case A_CLEAR:
   ;					GenPerformClear(action->u.val);
   ;	
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	call	far ptr @GenPerformClear$qus
	jmp	short @14@314
@14@242:
   ;	
   ;					break;
   ;				case A_ECHO:
   ;					GenPerformEcho(action->u.id);
   ;	
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+4]
	push	word ptr es:[bx+2]
	call	far ptr @GenPerformEcho$qnzc
	pop	cx
	pop	cx
	jmp	short @14@338
@14@266:
   ;	
   ;					break;
   ;				case A_HALT:
   ;					GenEnd();
   ;	
	push	cs
	call	near ptr @GenEnd$qv
   ;	
   ;					return (false);
   ;	
	xor	ax,ax
	jmp	short @14@410
@14@290:
   ;	
   ;				case A_SET:
   ;					GenPerformSet(action->u.val);
   ;	
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	call	far ptr @GenPerformSet$qus
@14@314:
	pop	cx
@14@338:
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx+8]
	mov	dx,word ptr es:[bx+6]
	mov	word ptr [bp+6],dx
	mov	word ptr [bp+8],ax
@14@362:
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	je	@@4
	jmp	@14@50
@@4:
   ;	
   ;					break;
   ;				}
   ;			}
   ;		return (true);
   ;	
	mov	ax,1
@14@410:
   ;	
   ;		}
   ;	
	pop	bp
	ret	
@GenPerform$qn11case_action	endp
@14@C462	label	word
	dw	@14@122
	dw	@14@194
	dw	@14@218
	dw	@14@242
	dw	@14@266
	dw	@14@290
   ;	
   ;	static void GenPerformAction(char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenPerformAction$qnzc	proc	far
	push	bp
	mov	bp,sp
   ;	
   ;		GenPatternMask(id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr @GenPatternMask$qnzc
	pop	cx
	pop	cx
   ;	
   ;		md = PDMA_SETUP_TRANSFER;
   ;	
	mov	word ptr DGROUP:md,1
   ;	
   ;		d[0] = dev.numTransfers;
   ;	
	mov	ax,word ptr DGROUP:_dev+18
	mov	word ptr DGROUP:d,ax
   ;	
   ;		d[2] = dev.direction;
   ;	
	mov	ax,word ptr DGROUP:_dev+20
	mov	word ptr DGROUP:d+4,ax
   ;	
   ;		d[3] = dev.autoRecycle;
   ;	
	mov	ax,word ptr DGROUP:_dev+22
	mov	word ptr DGROUP:d+6,ax
   ;	
   ;		d[4] = dev.clockSource;
   ;	
	mov	ax,word ptr DGROUP:_dev+24
	mov	word ptr DGROUP:d+8,ax
   ;	
   ;		d[5] = dev.buffSeg;
   ;	
	mov	ax,word ptr DGROUP:_dev+26
	mov	word ptr DGROUP:d+10,ax
   ;	
   ;		d[6] = dev.buffOff;
   ;	
	mov	ax,word ptr DGROUP:_dev+28
	mov	word ptr DGROUP:d+12,ax
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		}
   ;	
	pop	bp
	ret	
@GenPerformAction$qnzc	endp
   ;	
   ;	static void GenPerformClear(Word val) {
   ;	
	assume	cs:GEN_TEXT
@GenPerformClear$qus	proc	far
	push	bp
	mov	bp,sp
	push	si
   ;	
   ;		short	i;
   ;	
   ;		dev.aux[val] = 0;
   ;	
	mov	bx,word ptr [bp+6]
	shl	bx,1
	mov	word ptr DGROUP:_dev[bx+52],0
   ;	
   ;		md = PDMA_AUX_OUTPUT;
   ;	
	mov	word ptr DGROUP:md,6
   ;	
   ;		for (i=NUMELE(dev.aux)-1; i>=0; i--)
   ;	
	mov	si,2
	jmp	short @16@98
@16@50:
   ;	
   ;			d[i] = dev.aux[i];
   ;	
	mov	bx,si
	shl	bx,1
	mov	ax,word ptr DGROUP:_dev[bx+52]
	mov	bx,si
	shl	bx,1
	mov	word ptr DGROUP:d[bx],ax
	dec	si
@16@98:
	or	si,si
	jge	short @16@50
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		}
   ;	
	pop	si
	pop	bp
	ret	
@GenPerformClear$qus	endp
   ;	
   ;	static void GenPatternMask(char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenPatternMask$qnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,76
	push	si
	push	di
   ;	
   ;		Byte	huge *bblock;
   ;		Word	huge *wblock;
   ;		Link	*lk;
   ;		Byte	*mask;
   ;		Word	numRepeats, wild[WORD_BIT], numWild, preVal, i;
   ;		Word	postVal, j;
   ;		DWord	numPat, numTrans=0L, numCount, bitPos;
   ;	
	mov	word ptr [bp-32],0
	mov	word ptr [bp-30],0
   ;	
   ;	
   ;		bblock = (Byte *)MK_FP(dev.buffSeg, dev.buffOff);
   ;	
	mov	ax,word ptr DGROUP:_dev+26
	mov	dx,word ptr DGROUP:_dev+28
	mov	word ptr [bp-4],dx
	mov	word ptr [bp-2],ax
   ;	
   ;		wblock = (Word *)MK_FP(dev.buffSeg, dev.buffOff);
   ;	
	mov	ax,word ptr DGROUP:_dev+26
	mov	dx,word ptr DGROUP:_dev+28
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
   ;	
   ;		for (lk=SymbolFind(id)->val; lk != NULL; lk=lk->next) {
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	bx,ax
	mov	es,dx
	mov	ax,word ptr es:[bx+37]
	mov	dx,word ptr es:[bx+35]
	jmp	@17@1298
@17@50:
   ;	
   ;			switch (lk->u.action.qualify) {
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+17]
	or	ax,ax
	je	short @17@170
	cmp	ax,1
	je	short @17@194
	cmp	ax,2
	je	short @17@218
	jmp	@17@362
@17@170:
   ;	
   ;				case Q_NONE:
   ;					numRepeats = 1;
   ;	
	mov	di,1
	jmp	@17@362
@17@194:
   ;	
   ;					break;
   ;				case Q_REPEAT:
   ;					numRepeats = (Word)lk->u.action.val;
   ;	
	les	bx,dword ptr [bp-12]
	mov	di,word ptr es:[bx+19]
	jmp	@17@362
@17@218:
   ;	
   ;					break;
   ;				case Q_TIME:
   ;					if (dev.clockSource == DEV_CLOCK_EXTERNAL) {
   ;	
	cmp	word ptr DGROUP:_dev+24,0
	jne	short @17@266
   ;	
   ;						GenEnd();
   ;	
	push	cs
	call	near ptr @GenEnd$qv
   ;	
   ;						ErrorRunTime("%s TIME undefined when using external clock source", id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+779
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	add	sp,8
@17@266:
   ;	
   ;						}
   ;					if (lk->u.action.val < dev.unitTime) {
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+21]
	mov	dx,word ptr es:[bx+19]
	mov	word ptr [bp-44],dx
	mov	word ptr [bp-42],ax
	FILD	dword ptr [bp-44]
	FCOMP	qword ptr DGROUP:_dev+42
	FSTSW	word ptr [bp-44]
	FWAIT	
	mov	ax,word ptr [bp-44]
	sahf	
	jae	short @17@314
   ;	
   ;						GenPrint("Unit time of %.15g uS is indivisible (%s)\n", dev.unitTime, id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	FLD	qword ptr DGROUP:_dev+42
	add	sp,65528
	FSTP	qword ptr [bp-92]
	push	ds
	mov	ax,offset DGROUP:s@+830
	push	ax
	FWAIT	
	call	far ptr @GenPrint$qnzce
	add	sp,16
	jmp	short @17@170
@17@314:
   ;	
   ;						numRepeats = 1;
   ;						}
   ;					else {
   ;						numRepeats = lk->u.action.val / dev.unitTime;
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+21]
	mov	dx,word ptr es:[bx+19]
	mov	word ptr [bp-44],dx
	mov	word ptr [bp-42],ax
	FILD	dword ptr [bp-44]
	FDIV	qword ptr DGROUP:_dev+42
	call	far ptr F_FTOL@
	mov	di,ax
   ;	
   ;						if (numRepeats * dev.unitTime != lk->u.action.val)
   ;	
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+21]
	mov	dx,word ptr es:[bx+19]
	mov	word ptr [bp-44],dx
	mov	word ptr [bp-42],ax
	FILD	dword ptr [bp-44]
	mov	word ptr [bp-44],di
	mov	word ptr [bp-42],0
	FILD	dword ptr [bp-44]
	FMUL	qword ptr DGROUP:_dev+42
	FCOMPP
	FSTSW	word ptr [bp-44]
	FWAIT	
	mov	ax,word ptr [bp-44]
	sahf	
	je	short @17@362
   ;	
   ;							GenPrint("%s TIME specified not a multiple of unit time\n", id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+873
	push	ax
	call	far ptr @GenPrint$qnzce
	add	sp,8
@17@362:
   ;	
   ;						}
   ;					break;
   ;				}
   ;			mask = (Byte *)lk->u.action.mask;
   ;	
	mov	ax,word ptr [bp-10]
	mov	dx,word ptr [bp-12]
	mov	word ptr [bp-16],dx
	mov	word ptr [bp-14],ax
   ;	
   ;			bitPos = 0x8000;
   ;	
	mov	word ptr [bp-40],32768
	mov	word ptr [bp-38],0
   ;	
   ;			for (preVal=0, numWild=0, i=0; i<WORD_BIT; i++, mask++) {
   ;	
	mov	word ptr [bp-20],0
	mov	word ptr [bp-18],0
	mov	word ptr [bp-22],0
	jmp	short @17@578
@17@386:
   ;	
   ;				switch (*mask) {
   ;	
	les	bx,dword ptr [bp-16]
	mov	al,byte ptr es:[bx]
	mov	ah,0
	cmp	ax,42
	je	short @17@506
	cmp	ax,49
	je	short @17@482
	jmp	short @17@530
@17@482:
   ;	
   ;					case '1':
   ;						preVal += (Word)bitPos;
   ;	
	mov	ax,word ptr [bp-40]
	add	word ptr [bp-20],ax
	jmp	short @17@530
@17@506:
   ;	
   ;						break;
   ;					case '*':
   ;						wild[numWild++] = (Word)bitPos;
   ;	
	mov	ax,word ptr [bp-18]
	inc	word ptr [bp-18]
	shl	ax,1
	lea	bx,word ptr [bp-76]
	add	bx,ax
	mov	ax,word ptr [bp-40]
	mov	word ptr ss:[bx],ax
@17@530:
   ;	
   ;						break;
   ;					}
   ;				bitPos >>= 1;
   ;	
	mov	ax,word ptr [bp-38]
	mov	dx,word ptr [bp-40]
	shr	ax,1
	rcr	dx,1
	mov	word ptr [bp-40],dx
	mov	word ptr [bp-38],ax
	inc	word ptr [bp-22]
	inc	word ptr [bp-16]
@17@578:
	cmp	word ptr [bp-22],16
	jb	short @17@386
   ;	
   ;				}
   ;			numPat = (numWild == 0) ? (1) : (1UL << numWild);
   ;	
	cmp	word ptr [bp-18],0
	jne	short @17@650
	xor	dx,dx
	mov	ax,1
	jmp	short @17@674
@17@650:
	xor	dx,dx
	mov	ax,1
	mov	cl,byte ptr [bp-18]
	call	far ptr F_LXLSH@
@17@674:
	mov	word ptr [bp-28],ax
	mov	word ptr [bp-26],dx
   ;	
   ;			if ((numTrans+=numPat*numRepeats) > PAGE) {
   ;	
	mov	bx,di
	xor	cx,cx
	mov	ax,word ptr [bp-28]
	call	far ptr F_LXMUL@
	add	word ptr [bp-32],ax
	mov	ax,word ptr [bp-32]
	adc	word ptr [bp-30],dx
	mov	dx,word ptr [bp-30]
	cmp	dx,1
	jb	short @17@770
	ja	short @17@746
	or	ax,ax
	jbe	short @17@770
@17@746:
   ;	
   ;				GenEnd();
   ;	
	push	cs
	call	near ptr @GenEnd$qv
   ;	
   ;				ErrorRunTime(".ACTION '%s' overflows physical page boundary", id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+920
	push	ax
	call	far ptr @ErrorRunTime$qnzce
	add	sp,8
@17@770:
   ;	
   ;				}
   ;			for (numCount=0; numCount<numPat; numCount++) {
   ;	
	mov	word ptr [bp-36],0
	mov	word ptr [bp-34],0
	jmp	@17@1202
@17@794:
   ;	
   ;				postVal = preVal;
   ;	
	mov	ax,word ptr [bp-20]
	mov	word ptr [bp-24],ax
   ;	
   ;				for (bitPos=numCount, j=0; bitPos && j<numWild; bitPos>>=1, j++)
   ;	
	mov	ax,word ptr [bp-34]
	mov	dx,word ptr [bp-36]
	mov	word ptr [bp-40],dx
	mov	word ptr [bp-38],ax
	xor	si,si
	jmp	short @17@890
@17@818:
   ;	
   ;					if (bitPos & 0x0001)
   ;	
	mov	ax,word ptr [bp-40]
	and	ax,1
	or	ax,0
	je	short @17@866
   ;	
   ;						postVal += wild[j];
   ;	
	mov	ax,si
	shl	ax,1
	lea	bx,word ptr [bp-76]
	add	bx,ax
	mov	ax,word ptr ss:[bx]
	add	word ptr [bp-24],ax
@17@866:
	mov	ax,word ptr [bp-38]
	mov	dx,word ptr [bp-40]
	shr	ax,1
	rcr	dx,1
	mov	word ptr [bp-40],dx
	mov	word ptr [bp-38],ax
	inc	si
@17@890:
	mov	ax,word ptr [bp-40]
	or	ax,word ptr [bp-38]
	je	short @17@938
	cmp	si,word ptr [bp-18]
	jb	short @17@818
@17@938:
   ;	
   ;				if (dev.portMode == DEV_BYTE)
   ;	
	cmp	word ptr DGROUP:_dev+16,0
	jne	short @17@1082
   ;	
   ;					for (j=0; j<numRepeats; j++)
   ;	
	xor	si,si
	jmp	short @17@1034
@17@986:
   ;	
   ;						*bblock++ = postVal;
   ;	
	xor	dx,dx
	mov	ax,1
	push	ss
	pop	es
	lea	bx,word ptr [bp-4]
	call	far ptr F_PINA@
	mov	bl,byte ptr [bp-24]
	mov	es,dx
	xchg	bx,ax
	mov	byte ptr es:[bx],al
	inc	si
@17@1034:
	cmp	si,di
	jb	short @17@986
	jmp	short @17@1178
@17@1082:
   ;	
   ;				else
   ;					for (j=0; j<numRepeats; j++)
   ;	
	xor	si,si
	jmp	short @17@1154
@17@1106:
   ;	
   ;						*wblock++ = postVal;
   ;	
	xor	dx,dx
	mov	ax,2
	push	ss
	pop	es
	lea	bx,word ptr [bp-8]
	call	far ptr F_PINA@
	mov	bx,word ptr [bp-24]
	mov	es,dx
	xchg	bx,ax
	mov	word ptr es:[bx],ax
	inc	si
@17@1154:
	cmp	si,di
	jb	short @17@1106
@17@1178:
	add	word ptr [bp-36],1
	adc	word ptr [bp-34],0
@17@1202:
	mov	ax,word ptr [bp-34]
	mov	dx,word ptr [bp-36]
	cmp	ax,word ptr [bp-26]
	jae	@@5
	jmp	@17@794
@@5:
	jne	short @17@1274
	cmp	dx,word ptr [bp-28]
	jae	@@6
	jmp	@17@794
@@6:
@17@1274:
	les	bx,dword ptr [bp-12]
	mov	ax,word ptr es:[bx+25]
	mov	dx,word ptr es:[bx+23]
@17@1298:
	mov	word ptr [bp-12],dx
	mov	word ptr [bp-10],ax
	mov	ax,word ptr [bp-12]
	or	ax,word ptr [bp-10]
	je	@@7
	jmp	@17@50
@@7:
   ;	
   ;				}
   ;			}
   ;		dev.numTransfers = (Word)numTrans;
   ;	
	mov	ax,word ptr [bp-32]
	mov	word ptr DGROUP:_dev+18,ax
   ;	
   ;		}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
@GenPatternMask$qnzc	endp
   ;	
   ;	static void GenPerformEcho(char *id) {
   ;	
	assume	cs:GEN_TEXT
@GenPerformEcho$qnzc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
   ;	
   ;		Symbol	*sym;
   ;		Link	*cur;
   ;	
   ;		sym = SymbolFind(id);
   ;	
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr @SymbolFind$qnzc
	pop	cx
	pop	cx
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],dx
   ;	
   ;		for (cur=sym->val; cur!=NULL; cur=cur->next)
   ;	
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+37]
	mov	dx,word ptr es:[bx+35]
	jmp	short @18@98
@18@50:
   ;	
   ;			GenPrint("%s\n", cur->u.text);
   ;	
	les	bx,dword ptr [bp-8]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	push	ds
	mov	ax,offset DGROUP:s@+966
	push	ax
	call	far ptr @GenPrint$qnzce
	add	sp,8
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+25]
	mov	dx,word ptr es:[bx+23]
@18@98:
	mov	word ptr [bp-8],dx
	mov	word ptr [bp-6],ax
	mov	ax,word ptr [bp-8]
	or	ax,word ptr [bp-6]
	jne	short @18@50
   ;	
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenPerformEcho$qnzc	endp
   ;	
   ;	static void GenPerformSet(Word val) {
   ;	
	assume	cs:GEN_TEXT
@GenPerformSet$qus	proc	far
	push	bp
	mov	bp,sp
	push	si
   ;	
   ;		short	i;
   ;	
   ;		dev.aux[val] = 1;
   ;	
	mov	bx,word ptr [bp+6]
	shl	bx,1
	mov	word ptr DGROUP:_dev[bx+52],1
   ;	
   ;		md = PDMA_AUX_OUTPUT;
   ;	
	mov	word ptr DGROUP:md,6
   ;	
   ;		for (i=NUMELE(dev.aux)-1; i>=0; i--)
   ;	
	mov	si,2
	jmp	short @19@98
@19@50:
   ;	
   ;			d[i] = dev.aux[i];
   ;	
	mov	bx,si
	shl	bx,1
	mov	ax,word ptr DGROUP:_dev[bx+52]
	mov	bx,si
	shl	bx,1
	mov	word ptr DGROUP:d[bx],ax
	dec	si
@19@98:
	or	si,si
	jge	short @19@50
   ;	
   ;		pdma32(md, d);
   ;	
	push	ds
	mov	ax,offset DGROUP:d
	push	ax
	push	word ptr DGROUP:md
	call	far ptr @pdma32$qini
	add	sp,6
   ;	
   ;		}
   ;	
	pop	si
	pop	bp
	ret	
@GenPerformSet$qus	endp
   ;	
   ;	static void GenPrint(char *format, ...) {
   ;	
	assume	cs:GEN_TEXT
@GenPrint$qnzce	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
   ;	
   ;		va_list	args;
   ;	
   ;		va_start(args, format);
   ;	
	lea	ax,word ptr [bp+10]
	mov	word ptr [bp-4],ax
	mov	word ptr [bp-2],ss
   ;	
   ;		vprintf(format, args);
   ;	
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _vprintf
	add	sp,8
   ;	
   ;		va_end(args);
   ;		}
   ;	
	mov	sp,bp
	pop	bp
	ret	
@GenPrint$qnzce	endp
   ;	
   ;	void pdma32_interrupt(void) {
   ;	
	assume	cs:GEN_TEXT
@pdma32_interrupt$qv	proc	far
	push	bp
	mov	bp,sp
   ;	
   ;		stateChange = true;
   ;	
	mov	word ptr DGROUP:stateChange,1
   ;	
   ;		}
   ;	
	pop	bp
	ret	
@pdma32_interrupt$qv	endp
GEN_TEXT	ends
_BSS	segment word public 'BSS'
md	label	word
	db	2 dup (?)
d	label	word
	db	14 dup (?)
	?debug	C E9
_BSS	ends
_DATA	segment word public 'DATA'
s@	label	byte
	db	'@DOWN_KEY'
	db	0
	db	'@END_KEY'
	db	0
	db	'@ENTER_KEY'
	db	0
	db	'@ESC_KEY'
	db	0
	db	'@F10_KEY'
	db	0
	db	'@F1_KEY'
	db	0
	db	'@F2_KEY'
	db	0
	db	'@F3_KEY'
	db	0
	db	'@F4_KEY'
	db	0
	db	'@F5_KEY'
	db	0
	db	'@F6_KEY'
	db	0
	db	'@F7_KEY'
	db	0
	db	'@F8_KEY'
	db	0
	db	'@F9_KEY'
	db	0
	db	'@HOME_KEY'
	db	0
	db	'@LEFT_KEY'
	db	0
	db	'@PGDN_KEY'
	db	0
	db	'@PGUP_KEY'
	db	0
	db	'@RIGHT_KEY'
	db	0
	db	'@SPACE_KEY'
	db	0
	db	'@TAB_KEY'
	db	0
	db	'@UP_KEY'
	db	0
	db	'Referencing undefined .ACTION '
	db	39
	db	'%s'
	db	39
	db	0
	db	39
	db	'%s'
	db	39
	db	' is not of type .ACTION'
	db	0
	db	'GenCaseActionState1'
	db	0
	db	'GenCaseActionState2'
	db	0
	db	'GenCaseActionBeep'
	db	0
	db	'GenCaseActionClear'
	db	0
	db	'Referencing undefined .TEXT '
	db	39
	db	'%s'
	db	39
	db	0
	db	39
	db	'%s'
	db	39
	db	' is not of type .TEXT'
	db	0
	db	'GenCaseActionEcho1'
	db	0
	db	'GenCaseActionEcho2'
	db	0
	db	'GenCaseActionHalt'
	db	0
	db	'GenCaseActionSet'
	db	0
	db	'GenCaseKey1'
	db	0
	db	'GenCaseKey2'
	db	0
	db	'Referencing undefined .STATE '
	db	39
	db	'%s'
	db	39
	db	0
	db	39
	db	'%s'
	db	39
	db	' is not of type .STATE'
	db	0
	db	'GenCaseState1'
	db	0
	db	'GenCaseState2'
	db	0
	db	'PDMA-32 hardware not found'
	db	0
	db	'Unable to allocate sufficient far memory'
	db	0
	db	'Unable to set PDMA-32 timers'
	db	0
	db	'Unable to clear auxiliary outputs'
	db	0
	db	'Unable to enable hardware interrupt'
	db	0
	db	'Clock rate set to %.15g Hz'
	db	10
	db	0
	db	'%s TIME undefined when using external clock source'
	db	0
	db	'Unit time of %.15g uS is indivisible (%s)'
	db	10
	db	0
	db	'%s TIME specified not a multiple of unit time'
	db	10
	db	0
	db	'.ACTION '
	db	39
	db	'%s'
	db	39
	db	' overflows physical page boundary'
	db	0
	db	'%s'
	db	10
	db	0
_DATA	ends
GEN_TEXT	segment byte public 'CODE'
GEN_TEXT	ends
_s@	equ	s@
_KPressTab	equ	KPressTab
_pageTable	equ	pageTable
_caseState	equ	caseState
_caseStateLast	equ	caseStateLast
_caseKey	equ	caseKey
_caseKeyLast	equ	caseKeyLast
_caseStartup	equ	caseStartup
_caseStartupLast	equ	caseStartupLast
_stateChange	equ	stateChange
_md	equ	md
_d	equ	d
	public	@GenCaseAction$qnvnzc
	extrn	@SymbolFind$qnzc:far
	extrn	@ErrorSevere$qnzce:far
	extrn	_malloc:far
	extrn	@ErrorFatalList$qsnzc:far
	extrn	_strdup:far
	public	@GenCaseActionBeep$qnvus
	public	@GenCaseActionClear$qnvus
	public	@GenCaseActionEcho$qnvnzc
	public	@GenCaseActionHalt$qnv
	public	@GenCaseActionSet$qnvus
	public	@GenCaseKey$qnzc
	public	@GenCaseState$qnzc
	extrn	_bsearch:far
	extrn	_strcmp:far
	public	@GenStart$qv
	extrn	_dev:word
	extrn	@pdma32$qini:far
	extrn	@ErrorRunTime$qnzce:far
	extrn	_farmalloc:far
	extrn	F_PCMP@:near
	extrn	@KeyboardAvail$qv:far
	extrn	@KeyboardRead$qv:far
	extrn	_sound:far
	extrn	@JiffyDelay$qulnqv$v:far
	extrn	_nosound:far
	extrn	F_FTOL@:near
	extrn	F_LXLSH@:near
	extrn	F_LXMUL@:near
	extrn	F_PINA@:near
	extrn	_vprintf:far
	public	@pdma32_interrupt$qv
	end
