# Roadmap de NeuroOS

## Fase 0 - Base reproducible

- Inicializar Git y subir el primer estado del proyecto.
- Mantener fuera del repositorio objetos, ISO, logs y respaldos comprimidos.
- Documentar comandos de build y ejecucion.

## Fase 1 - Kernel observable

- Consolidar syscalls: `write`, `read`, `getpid`, `getticks`, `ps`, `kill`.
- Hacer que `SYS_EXIT` termine el proceso actual de forma real.
- Mejorar excepciones para imprimir vector, EIP y codigo de error antes de parar.
- Separar funciones comunes de impresion numerica para reducir duplicacion.

## Fase 2 - Scheduler adaptativo

- Agregar quantum por proceso.
- Medir ticks ejecutados, esperados y perdidos por ventana.
- Implementar aging controlado para evitar inanicion.
- Registrar decisiones del scheduler para depuracion.

## Fase 3 - Memory AI inicial

- Exponer estadisticas del PMM/VMM a la shell.
- Medir presion de memoria por proceso.
- Crear politicas simples: advertencia, throttling y preferencia de procesos.

## Fase 4 - Resiliencia

- Leer senales de CPU disponibles en entorno virtualizado.
- Crear una interfaz abstracta de health checks de hardware.
- Simular degradacion de nucleo/proceso antes de tocar MSR reales.
- Agregar modo degradado con politicas conservadoras.

## Fase 5 - Seguridad y separacion

- Revisar transiciones Ring 3 -> Ring 0.
- Definir stacks de usuario y kernel por proceso.
- Separar segmentos RX/RW en linker script.
- Reducir mapeo identity global conforme avance el VMM.

## Fase 6 - Salto de arquitectura

- Evaluar migracion a x86-64.
- Preparar boot path UEFI/limine o GRUB2 long mode.
- Redisenar estructuras dependientes de 32 bits.
