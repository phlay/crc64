%define ECMA_X64	0x42f0e1eba9ea3693
%define ECMA_X128	0x05f5c3c7eb52fab6
%define ECMA_X192	0x4eb938a7d257740e
%define ECMA_MU		0x578d29d06cc4f872

section .text

; Input:
;	RDI	start value for crc
;	RSI	pointer to input data
;	RDX	length of input
;
; Output:
;	RAX	CRC64-ECMA of the given data
;
	global	crc64

crc64:	mov		rax, rdi
	mov		ecx, edx	; ecx <- length modulo 16
	and		ecx, 0x0f
	jz		crc64_mainloop

	xor		rax, rax	; load ecx bytes from rsi into xmm0
	pxor		xmm0, xmm0
.load:	lodsb
	movq		xmm1, rax
	pslldq		xmm0, 1
	por		xmm0, xmm1
	loop		.load

	movq		xmm1, rdi	; xmm1 <- start crc
	mov		ecx, edx	; ecx  <- length modulo 8
	and		ecx, 0x07
	jz		.skip_shift
.shift:	pslldq		xmm1, 1		; xmm1 <- xmm1 << (length mod 8)
	loop		.shift
.skip_shift:

	test		dl, 0x08	; length mod 16 < 8
	jz		.skip_reduce
	pxor		xmm0, xmm1
	mov		rax, ECMA_X128
	movq		xmm1, rax
	pclmullqhqdq	xmm1, xmm0

.skip_reduce:
	pslldq		xmm0, 8		; shift-in 64 zero bits
	pxor		xmm0, xmm1
	call		reduce

	and		dl, 0xf0	; round length down to multiple of 16

	;
	; fallthrough to crc_go
	;


; Input:
;	RAX	start offset for crc
;	RSI	pointer to input data
;	RDX	length of input, with RDX = 0 modulo 16
;
; Output:
;	RAX	CRC64-ECMA of the given data
;
crc64_mainloop:
	add		rdx, rsi		; calculate end of data
	cmp		rsi, rdx
	jne		.init
	ret					; nothing to do, just exit

.init:	vmovdqa		xmm5, [endian]
	mov		rcx, ECMA_X128
	movq		xmm6, rcx
	mov		rcx, ECMA_X192
	movq		xmm7, rcx

	vmovdqu		xmm0, [rsi]		; load first data block
	pshufb		xmm0, xmm5		; convert to little-endian
	add		rsi, 16

	movq		xmm1, rax		; xmm0 <- xmm0 xor (rax << 64)
	pslldq		xmm1, 8
	pxor		xmm0, xmm1

.loop:	cmp		rsi, rdx
	je		.exit

	vpclmullqlqdq	xmm1, xmm0, xmm6	; xmm1 <- xmm0-lq * ECMA_X128
	vpclmulhqlqdq	xmm2, xmm0, xmm7	; xmm2 <- xmm0-hq * ECMA_X192
	vmovdqu		xmm0, [rsi]		; xmm0 <- next data block
	pshufb		xmm0, xmm5		; convert to little-endian
	pxor		xmm0, xmm1
	pxor		xmm0, xmm2
	add		rsi, 16
	jmp		.loop


.exit:	vpclmulhqlqdq	xmm1, xmm0, xmm6	; shift in 64 zero bits
	pslldq		xmm0, 8
	pxor		xmm0, xmm1


	;
	; fallthrough to reduce
	;

;
; Input:
;   XMM0	128bit CRC
;
; Output:
;   RAX		CRC reduced to 64bit
;
reduce:	vpsrldq		xmm5, xmm0, 8	; xmm5 <- xmm0 with lower qw 0
	vpslldq		xmm5, xmm5, 8

	mov		rax, ECMA_MU	; multiply by mu := floor( x^128 / p )
	movq		xmm1, rax
	pclmullqhqdq	xmm1, xmm0
	pxor		xmm1, xmm5	; mu was missing x^64, so correct manually

	; multiply by p (x^64 is missing but we use result modulo x^64 anyway)
	mov		rax, ECMA_X64
	movq		xmm2, rax
	pclmullqhqdq	xmm2, xmm1
	pxor		xmm0, xmm2	; add to input and return lower 64bit
	movq		rax, xmm0
	ret


section .data

	align	16

endian: db	0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08
	db	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
