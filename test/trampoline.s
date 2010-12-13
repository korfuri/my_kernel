[GLOBAL trampoline]

trampoline:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8]	; fct
	mov eax, [ebp+12]	; data

	mov ecx, esp		; old esp
	mov esp, [ebp+16]	; new esp
	push ecx		; save old esp
	push eax		; push data
	call ebx

	add esp, 4		; get rid of the data on stack
	pop ecx
	mov esp, ecx		; Restore old esp
	
	leave
	ret

