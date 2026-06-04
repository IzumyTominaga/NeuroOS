bits 32
global context_switch

; void context_switch(context_t* old, context_t* new);
context_switch:
    ; --- 1. GUARDAR ESTADO VIEJO ---
    mov ecx, [esp+4]      ; ecx = puntero a 'old'
    
    mov [ecx+0],  edi
    mov [ecx+4],  esi
    mov [ecx+8],  ebp
    mov [ecx+12], ebx
    mov [ecx+16], edx
    
    ; Guardar EIP (la direccion de retorno en el stack)
    mov eax, [esp]
    mov [ecx+28], eax
    
    ; Guardar EFLAGS
    pushf
    pop eax
    mov [ecx+36], eax
    
    ; Guardar ESP (el original antes de llamar a la funcion)
    mov eax, esp
    add eax, 4
    mov [ecx+40], eax

    ; --- 2. CARGAR ESTADO NUEVO ---
    mov ecx, [esp+8]      ; ecx = puntero a 'new'
    
    ; Cambiar el Stack Pointer
    mov esp, [ecx+40]
    
    ; Restaurar EFLAGS
    mov eax, [ecx+36]
    push eax
    popf
    
    ; Restaurar registros generales
    mov edi, [ecx+0]
    mov esi, [ecx+4]
    mov ebp, [ecx+8]
    mov ebx, [ecx+12]
    mov edx, [ecx+16]
    
    ; Saltar a la nueva instruccion (EIP)
    mov eax, [ecx+28]
    jmp eax
