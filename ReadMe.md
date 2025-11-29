# Bootloader ASM (boot.asm)

Ce fichier contient le point d’entrée de notre kernel ainsi que le **header Multiboot** compatible avec GRUB.
Il prépare également la **pile (stack)** et transfère ensuite l’exécution à la fonction `kernel_main()` du noyau C.

Cette implémentation suit principalement la documentation *Multiboot Specification* ainsi que le tutoriel **Bare Bones** d’OSDev.

---

## 📌 1. Mode 32 bits

```asm
BITS 32
```

Notre kernel est conçu pour être exécuté directement en **mode protégé 32 bits**, car GRUB configure déjà la machine dans ce mode avant de transférer le contrôle au kernel.

---

## 📌 2. Header Multiboot

Pour que GRUB reconnaisse et charge correctement notre kernel, nous devons fournir un **header Multiboot** contenant :

* une valeur magique (`MAGIC`) que GRUB recherche,
* des flags indiquant nos besoins,
* un checksum permettant la validation du header.

```asm
%define ALIGN      (1 << 0)      ; Demande un alignement du module sur 4 octets
%define MEMINFO    (1 << 1)      ; Demande à GRUB de passer les informations mémoire
%define FLAGS      (ALIGN | MEMINFO)

%define MAGIC      0x1BADB002    ; Signature Multiboot obligatoire
%define CHECKSUM   -(MAGIC + FLAGS)
```

Les trois valeurs doivent satisfaire :

```
MAGIC + FLAGS + CHECKSUM ≡ 0 (mod 2^32)
```

---

## 📌 3. Section Multiboot

```asm
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
```

Le header doit être :

* placé dans les **8 premiers Ko** du fichier binaire,
* **aligné sur 4 octets**.

GRUB lit cette section pour vérifier que le kernel est bien conforme à la spécification Multiboot.

---

## 📌 4. Création de la stack (pile)

```asm
section .bss
align 16
stack_bottom:
    resb 16384        ; Réserve 16 KB pour la pile
stack_top:
```

La section **.bss** contient des données non initialisées.
Ici, nous réservons **16 KB de mémoire** pour servir de pile (stack).

Important :
La pile **descend**, donc `ESP` doit commencer à `stack_top`.

---

## 📌 5. Point d’entrée du kernel

```asm
section .text
global _start
_start:
    mov esp, stack_top
    call kernel_main
```

### `_start` est le véritable point d’entrée du kernel

Après avoir chargé le fichier ELF, **GRUB saute directement à `_start`**.

### Initialisation de la pile

On positionne le registre `ESP` en haut de la pile (`stack_top`).

### Appel du kernel C

`kernel_main()` est la fonction principale écrite en C qui contiendra la logique du noyau.

---

## 📌 6. Boucle d’arrêt (sécurité)

```asm
.hang:
    cli
    hlt
    jmp .hang
```

Cette boucle infinie sert de **fail-safe**.
Si jamais `kernel_main()` retournait (ce qui ne devrait jamais arriver dans un OS), le CPU :

* désactive les interruptions (`cli`),
* se met en pause (`hlt`),
* et reste bloqué dans cette boucle.

Ce comportement évite l’exécution d’instructions invalides et permet un arrêt propre.

---

## 📚 Ressources

* **OSDev – Bare Bones**
  [https://wiki.osdev.org/Bare_Bones](https://wiki.osdev.org/Bare_Bones)

---

Ce fichier constitue la toute première étape du boot :
✔ Déclarer un header Multiboot
✔ Initialiser la stack
✔ Transférer le contrôle au kernel C

Il travaille de pair avec le linker et la partie C du noyau, présentés dans les sections suivantes du projet.
