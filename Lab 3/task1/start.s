section .bss ;uninitialized data
 user_input: resb 256
section .data ;initialized global vars
 arg_len:dd 0 ;pointers to main args
 args:dd 0
 letter: db 0
 index: dd 0 ;used to end loop
 line: db 10 ;new line char
 Outfile:dd 1 ;default output
 Infile:dd 0 ;default input
 length: dd 0
 msg:db "enter to encode :)",10,0
 section .text ;code is here
  global _start ;so func is recognized outside encoder
  extern strlen ; "importing" strlen
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
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
main:
 ;prep args from stack
 mov eax, dword[esp+4] ;bringing argc
 mov dword[arg_len],eax ;storing argc in pointer
 mov eax, dword[esp+8] ;bringing argv
 mov dword[args],eax ;storing a pointer to argvs
 while_loop: ;our loop
  mov ecx,dword[args] 
  mov edx,dword[index]
  mov ecx, dword[ecx+4*edx] ;msg arg 3 for write argv[index]
  push ecx ;to check length
  call strlen ;length is now in eax
  add esp,4 ;default pointer to stack
  mov dword[length],eax
  print:
  mov eax,4
  mov ebx,1 ; fd for stdout arg 2 for write 
  mov edx,dword[index]
  mov ecx,dword[args]
  mov ecx,dword[ecx+4*edx]
  mov edx,dword[length] ;arg4 for write
  int 0x80 ;kernel does sys write  
  ;printig a new line
  mov eax,4
  mov ebx,1
  mov ecx,line
  mov edx,1 ;new line char is 1 length
  int 0x80
 check_file:
  mov edx,dword[index]
  mov ecx,dword[args]
  mov ecx,dword[ecx+4*edx] ;ecx has arg[i]
  cmp byte[ecx],'-'
  jne check_end
  cmp byte[ecx+1],'i'
  je input_file
  cmp byte[ecx+1],'o'
  je output_file
  jne check_end
  input_file:
   mov eax,5 ;open file num
   mov ebx,ecx
   add ebx, 2
   mov ecx,0
   int 0x80
   mov dword[Infile],eax ;need to check for error
   jmp check_end
  output_file:
 mov eax,5 ;open file num
   mov ebx,ecx
   add ebx, 2
   mov ecx,01101o ;for writee only or create if doesnt exist
   mov edx ,0644o ;permissions
   int 0x80
   mov dword[Outfile],eax
  check_end: ;checking if we're done printing
  mov eax,dword[index]
  add eax,1
  mov dword[index],eax
  cmp eax,dword[arg_len]
  je encode ;if we got to the end of the args we exit
  jne while_loop ;go back to start of loop
  encode:
  ;prompt for user
  mov eax,4
  mov ebx,1
  mov ecx,msg
  mov edx,19
  int 0x80
  ;;end of prompt for user 
   encoding_loop:
   mov eax, 3 ;num for read
   mov ebx, dword[Infile]
   mov ecx,letter
   mov edx,1
   int 0x80
   cmp eax,0 ;if its the end of the input
   jle exit
   ;;we want to encode all from A to z
   cmp byte[letter],'A' 
   jl print_no_change
   cmp byte[letter],'z'
   jg print_no_change
   jmp print_encoded
   
  print_no_change:
  mov eax,4
  mov ebx,dword[Outfile]
  mov ecx,letter
  mov edx,1
  int 0x80
  jmp return; goes back
  print_encoded:
  add byte [letter],1
  mov eax,4
  mov ebx, dword[Outfile]
  mov ecx,letter
  mov edx,1
  int 0x80
  jmp return
return:
   jmp encoding_loop
 exit: ;exit function
    mov eax,1 ;number for exit sys call
    xor ebx,ebx ;storing 0 in ebx
    int 0x80 ;kernel does sys call