[GLOBAL sys_write]
[GLOBAL sys_blu]
[GLOBAL sys_exit]
[GLOBAL sys_switch_to_user_mode]

sys_write:
	mov eax, 1
	int 0x80
	ret

sys_blu:
	mov eax, 2
	int 0x80
	ret

sys_exit:
	mov eax, 3
	int 0x80
	ret

sys_switch_to_user_mode:
	mov eax, 4
	int 0x80
	ret
