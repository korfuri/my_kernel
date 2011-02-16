[GLOBAL switch_thread_asm]
[GLOBAL new_thread_asm]
[EXTERN thread_entry]
[EXTERN schedule_C]
[GLOBAL schedule]
[EXTERN	current_thread]
[EXTERN segment_user_stack]
[EXTERN segment_user_code]
[GLOBAL switch_to_user_mode]
	
new_thread_asm:
	push ebp
	mov ebp, esp
	
	pushad			; Save everything
	pushfd

	mov edx, [ebp+8]	; New ESP
	mov ecx, [ebp+12]	; Future EIP
				; ebp+16 is where to save old esp
	mov ebx, [ebp+20]	; "data"

	mov eax, esp 		; Prepare to return the old stack
	mov [ebp+16], eax	; Save old stack somewhere
	
	mov esp, edx		; Start working on the new stack, yay
	push ebx		; "Data"
	push ecx		; Future EIP
	push eax		; Old stack

	push 0			; 4 bytes junk so thread_entry can read its arguments
	jmp thread_entry

switch_thread_asm:
	push ebp
	mov ebp, esp

	pushad			; Save everything
	pushfd

	mov eax, [ebp+12]	; Get a pointer where to save the old esp
	cmp eax, 0		; If we have a non-null pointer
	je .nosave		
	mov [eax], esp		; Save esp
.nosave:
	mov esp, [ebp+8]	; Start working with the new ESP

	popfd			; Restore everything
	popad

	leave
	ret

switch_to_user_mode:
	cli
	xor eax, eax
	mov ax, [segment_user_stack]
	or ax, 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push eax			; ss
	push esp		; esp
	pushf			; eflags

	pop eax			; modify eflags to enable interrupts
	or eax, 0x200
	push eax

	mov eax, [segment_user_code]
	or ax, 3
	push eax			; cs
	push continue

	iret
continue:
