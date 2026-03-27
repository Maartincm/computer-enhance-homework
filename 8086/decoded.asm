mov dx, 6
mov bp, 1000
mov si, 0
mov [bp + si], si
add si, word 2
cmp si, dx
jnz $-7
mov [bp + si], si
add si, word 2
cmp si, dx
jnz $-7
mov [bp + si], si
add si, word 2
cmp si, dx
jnz $-7
mov bx, 0
mov si, dx
sub bp, word 2
add bx, [bp + si]
sub si, word 2
jnz $-5
add bx, [bp + si]
sub si, word 2
jnz $-5
add bx, [bp + si]
sub si, word 2
jnz $-5
