bits 16

; Start image after one row, to avoid overwriting our code!
mov bp, 64*4
mov al, 255

; Draw the solid rectangle red/blue/alpha
mov dx, 64
mov di, 8
y_loop_start:
	sub di, 1

	mov cx, 64
	mov si, 8
	x_loop_start:
		sub si, 1
		mov bl, al

		cmp bl, 255
		je x_module_with_sub

		add bl, dl
		add bl, dl
		add bl, dl
		mov byte [bp + 0], bl  ; Red
		mov bl, al
		add bl, cl
		add bl, cl
		add bl, cl
		mov byte [bp + 1], bl  ; Green
		mov bl, al
		add bl, dl
		add bl, cl
		mov byte [bp + 2], bl  ; Blue
		mov byte [bp + 3], 255 ; Alpha
		mov bl, 1
		cmp bl, 0
		jnz x_modulate_with_add

		x_module_with_sub:
		sub bl, cl
		sub bl, cl
		sub bl, cl
		mov byte [bp + 0], bl  ; Red
		mov bl, al
		sub bl, dl
		sub bl, dl
		sub bl, dl
		mov byte [bp + 1], bl  ; Green
		mov bl, al
		sub bl, cl
		sub bl, dl
		mov byte [bp + 2], bl  ; Blue
		mov byte [bp + 3], 255 ; Alpha

		mov bl, 1
		cmp bl, 0
		jnz y_loop_start_step_1_skip
		y_loop_start_step_1:
		mov bl, 1
		cmp bl, 0
		jnz y_loop_start
		y_loop_start_step_1_skip:

		x_modulate_with_add:

		cmp si, 0
		jnz x_skip_toggle
		mov si, 8
		cmp al, 0
		je color_toggle
		mov al, 0
		jnz dont_color_toggle
		color_toggle:
		mov al, 255
		dont_color_toggle:
		x_skip_toggle:


		add bp, 4
		loop x_loop_start

	cmp di, 0
	jnz y_skip_toggle
	mov di, 8
	cmp al, 0
	je y_color_toggle
	mov al, 0
	jnz y_dont_color_toggle
	y_color_toggle:
	mov al, 255
	y_dont_color_toggle:
	y_skip_toggle:

	sub dx, 1
	jnz y_loop_start_step_1

; Add the line rectangle green
; mov bp, 64*4 + 4*64 + 4
; mov bx, bp
; mov cx, 62
; outline_loop_start:
;
; 	mov byte [bp + 1], 255 ; Top line
; 	mov byte [bp + 61*64*4 + 1], 255 ; Bottom line
; 	mov byte [bx + 1], 255 ; Left line
; 	mov byte [bx + 61*4 + 1], 255 ; Right  line
;
; 	add bp, 4
; 	add bx, 4*64
;
; 	loop outline_loop_start
