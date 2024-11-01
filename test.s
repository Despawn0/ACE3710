	.ifndef test_s
	.define test_s

	.macro t
	.word $1
	.endmacro

	.macro jl rTrash, lbl
@val = 1;
	lui rTrash, >lbl
	ori rTrash, <lbl
	juc rTrash
	.endmacro

	.macro sw rSrc, rTrash, lbl
	lui rTrash, >lbl & $00ff
	ori rTrash, <lbl
	stor rSrc, rTrash
	t
	.endmacro
	
	.segment "CODE"
add:
	lui r0, 0
	addi r0, 1
	sw r0, r1, 0 + 1
@loop:
	jl r1, @loop	;that is all folks
	
	.endif
