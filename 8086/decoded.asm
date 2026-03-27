mov ax, 10
mov bx, 10
mov cx, 10
cmp bx, cx
je $+7
sub bx, word 5
jb $+5
sub cx, word 2
loopnz $-17
cmp bx, cx
je $+7
add ax, word 1
jp $+7
sub bx, word 5
jb $+5
sub cx, word 2
loopnz $-17
cmp bx, cx
je $+7
add ax, word 1
jp $+7
sub cx, word 2
loopnz $-17
cmp bx, cx
je $+7
add ax, word 1
jp $+7
sub bx, word 5
jb $+5
loopnz $-17
