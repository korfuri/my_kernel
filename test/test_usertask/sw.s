[GLOBAL switch_task_asm]
[GLOBAL new_task_asm]
[EXTERN coroutine_entry]

new_task_asm:	
	mov ecx, [esp+8]	; Future EIP
	mov edx, [esp+4]	; New ESP
	pushad			; Save everything
	pushfd
	mov eax, esp 		; Prepare to return the old stack
	mov esp, edx		; Start working on the new stack, yay
	push ecx		; Future EIP
	push eax		; Old stack
	push 0			; junk data
	push coroutine_entry	; At `ret', continue with the given EIP
	ret			; Continue

switch_task_asm:
	push ebp
	mov ebp, esp

	pushad
	pushfd

	mov esp, [ebp+8]

	popfd
	popad

	leave
	ret
