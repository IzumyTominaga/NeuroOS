bits 32

; Handler de int 0x80 (syscall gate)
; Al entrar: eax=syscall_num, ebx=arg1, ecx=arg2, edx=arg3
; El CPU ya pusheo: ss, esp, eflags, cs, eip

global syscall_stub
extern syscall_handler_c

syscall_stub:
    ; Guardar registros
    push edx
    push ecx
    push ebx
    push eax
    ; Cargar segmentos de kernel
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    ; Llamar al handler C
    call syscall_handler_c
    ; Limpiar stack
    add esp, 16
    ; Retornar a modo usuario via iret
    iret

; jump_to_usermode(entry, user_stack)
; Salta a codigo de usuario en ring3
; entry      = [esp+4]
; user_stack = [esp+8]
global jump_to_usermode
jump_to_usermode:
    mov eax, [esp+4]   ; entry point
    mov ecx, [esp+8]   ; user stack

    ; Segmentos de usuario: 0x18|3 y 0x20|3
    ; (GDT index 3 y 4, con RPL=3)
    mov bx, 0x23       ; user data segment (GDT[4] | ring3)
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    ; Construir frame de iret para ring3:
    ; SS, ESP, EFLAGS, CS, EIP
    push 0x23          ; SS  = user data
    push ecx           ; ESP = user stack
    pushfd
    or dword [esp], 0x200   ; IF=1 (interrupciones activas en ring3)
    push 0x1B          ; CS  = user code (GDT[3] | ring3)
    push eax           ; EIP = entry point
    iret
