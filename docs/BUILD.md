# Build y ejecucion

## Requisitos en WSL/Ubuntu

- `gcc` con soporte `-m32`
- `nasm`
- `ld`
- `grub-mkrescue`
- `xorriso`
- `qemu-system-i386`

## Compilar

```bash
cd ~/neuro-os
make
```

El build genera:

```text
neuro-os.bin
neuro-os.iso
```

## Ejecutar

```bash
make run
```

## NeuroShell

Comandos disponibles:

```text
help
neuro
ps
ticks
clear
kill <pid>
```

## Nota actual

El kernel compila como objetivo freestanding de 32 bits por ahora. La vision del repositorio apunta a x86-64, pero el camino tecnico sano es endurecer primero la base actual: interrupciones, syscalls, VMM/PMM, scheduler y telemetria.
