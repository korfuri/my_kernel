[GLOBAL sys_write]
[GLOBAL sys_blu]

sys_write:
	mov eax, 1
	int 0x80
	ret

sys_blu:
	mov eax, 2
	int 0x80
	ret
