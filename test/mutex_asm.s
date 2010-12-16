[GLOBAL mutex_lock]
[GLOBAL mutex_unlock]
[GLOBAL mutex_trylock]
[EXTERN schedule]

mutex_lock:
	mov ecx, [esp+4]
	mov eax, 1
.loop:
	xchg [ecx], eax
	test eax, eax
	jz .end
	call schedule
	jmp .loop
.end:
	ret
	
mutex_unlock:
	mov ecx, [esp+4]
	mov eax, 0
	xchg [ecx], eax
	ret

mutex_trylock:
	mov ecx, [esp+4]
	mov eax, 1
	xchg [ecx], eax
	ret
