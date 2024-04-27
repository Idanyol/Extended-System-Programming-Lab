section .text
    global _start
    global system_call
    
    extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop


system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller





;---------------------ass1--------------------------
section .data
    argc: dd 0
    newline: db 10
    infile: dd 0
    outfile: dd 1
    buff: db 1
    error_c: db "Can't open file!", 10
    error_len: equ $ - error_c
    
main: 
    mov dword [argc], ecx
    mov edi, 0 ; counter to argv
    
Next: 
    mov edx, [esi+4*edi]
    cmp byte [edx],'-'  ;if dont start with '-', just print
    jne print_arg

change_file:
    inc edx ;go to the sec char
    cmp byte [edx],'i' ;if input file
    je change_input
    cmp byte [edx], 'o' ;if output file
    je change_output

change_input:
    inc edx  ;go to the first char of file name
    push 0644; premmisions
    push 0   ; read
    push edx ;file name
    push 5   ;open
    call system_call
    add esp,16 
    
    cmp eax, 0  ; if 0, does not success                          
    jl print_error 
    mov dword [infile], eax ;
   
    jmp print_arg
   

change_output:
    inc edx
    push 0777
    push 0x41
    push edx
    push 0x5 
    call system_call
    add esp,16

    cmp eax, 0                            
    jl print_error
    mov dword[outfile],eax
    
   
    

print_arg:
    push dword [esi + 4 * edi] ; push av[i] (i=0 first)
    call strlen ; eax = av[i].len
    add esp, 4
    push eax ; the arg[i] len
    push dword [esi + 4 * edi] ; the arg[i]
    push 1
    push 4 
    call system_call ; print argv[i]
    add esp, 16 ; “remove” printf arguments

    ;print new line
    push 1   
    push newline
    push 1
    push 4
    call system_call
    add esp, 16  

    inc edi 
    dec dword[argc] ; dec. arg counter
    jnz Next ; loop if not yet zero
  
encoder: 
    ; read a char from unput file
    push 1 ;len of read chars
    push buff ;dest for what we read
    push dword[infile] ;source to read from
    push 3 ; READ command
    call system_call
    add esp, 16

    ;check if end of file
    cmp eax,0
    je finish;if EOF exit the program

    ;encoder
    cmp byte [buff], 'A' ;if smaller than 'A' val, just print
    jl p_char
    cmp byte [buff], 'z' ;if larger than 'A' val, just print
    jg p_char
    add byte[buff], 1    ;encoder: +1

p_char:
    push 1               ; which size to write
    push buff            ; what to write
    push dword[outfile]  ;where to write
    push 4
    call system_call
    add esp, 16
    jmp encoder ;till EOF

finish:
    ; close infile
    mov eax,6
    mov ebx, [infile]
    int 0x80
   
    ;close outfile
    mov eax,6
    mov ebx, [outfile]
    int 0x80
    ;exit the program
    mov eax, 1      ;sys_exit
    mov ebx, 0      ;success
    int 0x80

print_error:
    mov eax, 4                              ;4 = sys_Write.
    mov ebx, 1                              ;1 = stdout.
    mov ecx, error_c                     ;ecx = the value to print.
    mov edx, error_len                      ;edx = the length.
    int 0x80
    jmp finish


    

    

