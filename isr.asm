extern syscall_handler_c
extern proc_schedule
extern tick_and_schedule
extern keyboard_handler

global syscall_handler
global gdt_flush
global idt_flush

global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10
global isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20, isr21
global irq0, irq1, irq2, irq3

; MANEJADOR DE SYSCALL
syscall_handler:
    cli
    push edx
    push ecx
    push ebx
    push eax

    push esp
    call syscall_handler_c
    add esp, 4

    pop eax
    pop ebx
    pop ecx
    pop edx
    sti
    iretd

; MANEJADOR DEL RELOJ (IRQ0)
irq0:
    cli
    pusha
    mov al, 0x20
    out 0x20, al
    call tick_and_schedule
    popa
    sti
    iretd

; MANEJADOR DEL TECLADO (IRQ1)
irq1:
    cli
    pusha               ; Preservar el estado de los registros de la tarea actual
    
    call keyboard_handler ; Llamar a la rutina de lectura en C
    
    mov al, 0x20        ; Avisar fin de interrupcion al PIC maestro
    out 0x20, al
    
    popa
    sti
    iretd

irq2:
irq3:
    cli
    push eax
    mov al, 0x20
    out 0x20, al
    pop eax
    sti
    iretd

; EXCEPCIONES Y FLUSHES

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

%macro ISR_NOERRCODE 1
isr%1:
    cli
    hlt
    jmp isr%1
%endmacro

%macro ISR_ERRCODE 1
isr%1:
    cli
    hlt
    jmp isr%1
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
