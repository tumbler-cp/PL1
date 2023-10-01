section .text
 
 
; Принимает код возврата и завершает текущий процесс
exit: 
    mov rax, 60
    syscall 

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
    xor rax, rax
.cnt_loop:
    cmp rdi, rax
    jz  .end
    inc rax
    jmp .cnt_loop
.end:
    ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
    push rdi
    call string_length
    mov rsi, rdi
    mov rdx, rax
    mov rdi, 1
    mov rax, 1
    syscall
    pop rdi
    ret

; Принимает код символа и выводит его в stdout
print_char:
    push rdi
    mov rsi, rdi
    mov rdi, 1
    mov rdx, 1
    mov rax, 1
    syscall
    pop rdi
    ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
    mov rdi, 0xA
    call print_char
    ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
    push rdi
    push r8
    xor r8, r8
    mov rax, rdi
    mov rcx, 10
.div_loop:
    xor rdx, rdx
    div rcx
    add rdx, '0'
    push rdx
    inc r8
    cmp rax, 0
    jnz .div_loop
.print:
    pop rdi
    call print_char
    dec r8
    cmp r8, 0
    jnz .print
    pop r8
    pop rdi
    ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:
    push rdi
    test rdi, rdi
    jns .print_u
    mov rdi, '-'
    call print_char
    pop rdi
    push rdi
    neg rdi
.print_u:
    call print_uint
    pop rdi
    ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
    xor rax, rax
    xor rcx, rcx
.main_loop:
    mov al, byte[rdi + rcx]
    mov ah, byte[rsi + rcx]
    cmp al, ah
    jne .neq
    cmp al, 0
    je .eq
    inc rcx
    jmp .main_loop
.eq:
    mov rax, 1
.neq:
    ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
    xor rax, rax
    sub rsp, 8
    xor rdi, rdi
    mov rdx, 1
    syscall
    cmp rax, 0
    jle .error
    pop rax
    ret 
.error:
    pop rax
    mov rax, 0
    ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    push rdi
    push rsi
    xor rdx, rdx
.spaces:
    call read_char
    cmp al, 0x9
    je .spaces
    cmp al, 0xA
    je .spaces
    cmp al, 0x20
    je .spaces
    pop rsi
    pop rdi
.word:
    cmp al, 0x9
    je .fin
    cmp al, 0xA
    je .fin
    cmp al, 0x20
    je .fin
    test rax, rax
    jz .fin
    cmp rdx, rsi
    jg .fin_err
    mov [rdi+rdx], al
    inc rdx
    push rdx
    push rdi
    call read_char
    pop rdi
    pop rdx
    jmp .word
.fin_err:
    xor rax, rax
    xor rdx, rdx
    ret
.fin:
    mov byte[rdi + rdx], 0
    mov rax, rdi
    ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    push rbx
    xor rax, rax
    xor rdx, rdx
.loop:
	mov bl, byte[rdi+rdx]
	test bl, bl
	jz .fin
	cmp bl, '0'
	jl .fin_err
	cmp bl, '9'
	jg .fin_err
	sub rbx, '0'
	imul rax, 10
	add rax, rbx
	inc rdx
	jmp .loop
.fin_err:
	cmp rax, 0
	jnz .fin
	xor rdx, rdx
.fin:
	pop rbx
	ret


; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int: 
    cmp byte[rdi], '-'
    jne parse_uint
    inc rdi
    call parse_uint
	test rdx, rdx
	jz .end
    inc rdx
    neg rax
.end:
    ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
    push rdi
    push rsi
    push rdx
    call string_length
    xor rcx, rcx
    pop rdx
    pop rsi
    pop rdi
    inc rax
    cmp rax, rdx
    jg .err
.loop:
    cmp rax, rcx
	je .end
	mov dl, byte [rdi + rcx]
	mov [rsi + rcx], dl
	inc rcx
    jmp .loop	
.end:
    ret    
.err: 
    xor rax, rax
    ret
