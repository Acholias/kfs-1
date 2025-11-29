BITS 32									;Charge le kernel sur une architecture 32

										;Flags multiboot pour generer un header multiboot, pour savoir comment charger le kernel
%define ALIGN      (1 << 0)				;align le kernel sur 4octets
%define MEMINFO    (1 << 1)				;Demande à GRUB de fournir la carte mémoire
%define FLAGS      (ALIGN | MEMINFO)	;combine les deux define du dessus
%define MAGIC      0x1BADB002			;permet à GRUB de reconnaitre le kernel pour le boot
%define CHECKSUM   -(MAGIC + FLAGS)		;permet la validation du header par GRUB

section .multiboot						;ecrire le header dans la section multiboot
align 4									;GRUB lis les 3 valeurs au début du kernel
    dd MAGIC							;Dans les les premiers 8ko du binaire tous est alignée sur 4octets 
    dd FLAGS
    dd CHECKSUM

section .bss							;la section bss est la section des données non initialisé
align 16
stack_bottom:
    resb 16384							;on réserve une stack de 16KB (16384 bytes)
stack_top:

section .text							;point d'entrée du kernel pour 
global _start							;GRUB va ensuite jmp sur le _start après avoir charger le fichier elf
_start:
    mov	 esp, stack_top					;initialise la stack (architecture 32)
    call kernel_main					; appel la fonction main du kernel

.hang:									;Boucle pour que le CPU jmp en boucle
    cli									;désactive les interuptions
    hlt									;stop le CPU en cas de return dans le kernel_main
    jmp .hang
