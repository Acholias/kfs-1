# 🔧 Fonctions Assembleur - KFS-1

Ce document explique en détail les différentes fonctions assembleur utilisées dans le projet KFS-1. Ces fonctions constituent la base bas-niveau du kernel et permettent l'interface entre le bootloader, le matériel et le code C.

---

## 📋 Table des matières
1. [Bootloader (boot.asm)](#1-bootloader-bootasm)
2. [ft_memcpy](#2-ft_memcpy)
3. [ft_memset](#3-ft_memset)
4. [ft_strlen](#4-ft_strlen)

---

## 1. Bootloader (boot.asm)

### 📌 Vue d'ensemble
Le bootloader est le point d'entrée de notre kernel. Il contient le **header Multiboot** compatible avec GRUB et initialise l'environnement d'exécution avant de transférer le contrôle au code C.

### 🔍 Code complet
```asm
BITS 32
extern kernel_main

%define ALIGN      (1 << 0)
%define MEMINFO    (1 << 1)
%define FLAGS      (ALIGN | MEMINFO)
%define MAGIC      0x1BADB002
%define CHECKSUM   -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
_start:
    mov     esp, stack_top
    call    kernel_main
    cli
.hang:
    hlt
    jmp     .hang
```

### 📖 Explication détaillée

#### Mode 32 bits
```asm
BITS 32
```
Notre kernel s'exécute en **mode protégé 32 bits**. GRUB configure déjà le CPU dans ce mode avant de transférer le contrôle.

#### Header Multiboot
```asm
%define MAGIC      0x1BADB002    ; Signature Multiboot obligatoire
%define FLAGS      (ALIGN | MEMINFO)
%define CHECKSUM   -(MAGIC + FLAGS)
```

Le header Multiboot permet à GRUB de reconnaître notre kernel. Il doit satisfaire :
```
MAGIC + FLAGS + CHECKSUM ≡ 0 (mod 2³²)
```

**Flags utilisés :**
- `ALIGN (1 << 0)` : Demande l'alignement des modules sur 4 octets
- `MEMINFO (1 << 1)` : GRUB fournit les informations sur la mémoire disponible

#### Section Multiboot
```asm
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
```

Cette section doit être :
- Placée dans les **8 premiers Ko** du fichier binaire
- **Alignée sur 4 octets**

GRUB scanne cette zone pour valider le kernel.

#### Création de la pile (stack)
```asm
section .bss
align 16
stack_bottom:
    resb 16384        ; Réserve 16 KB
stack_top:
```

La section `.bss` contient les données non initialisées. Nous réservons **16 KB** pour la pile.

**⚠️ Important :** La pile descend en mémoire, donc `ESP` pointe vers `stack_top`.

#### Point d'entrée
```asm
_start:
    mov     esp, stack_top    ; Initialise le pointeur de pile
    call    kernel_main       ; Appelle la fonction C principale
```

1. `ESP` est positionné au sommet de la pile
2. On appelle `kernel_main()` qui contient la logique du noyau en C

#### Boucle d'arrêt (fail-safe)
```asm
    cli                ; Désactive les interruptions
.hang:
    hlt                ; Met le CPU en pause
    jmp     .hang      ; Boucle infinie
```

Si `kernel_main()` retourne (ce qui ne devrait jamais arriver), le CPU :
- Désactive les interruptions (`cli`)
- Se met en pause (`hlt`)
- Reste bloqué dans une boucle infinie

Cela évite l'exécution d'instructions invalides.

---

## 2. ft_memcpy

### 📌 Prototype
```c
void *ft_memcpy(void *dest, const void *src, size_t n);
```

### 🎯 Fonction
Copie `n` octets depuis `src` vers `dest`. Retourne `dest`.

### 🔍 Code complet
```asm
section .text
    global ft_memcpy

ft_memcpy:
    push    ebp                ; Sauvegarde le base pointer
    mov     ebp, esp           ; Établit le stack frame
    push    esi                ; Sauvegarde esi
    push    edi                ; Sauvegarde edi
    
    mov     edi, [ebp + 8]     ; edi = dest
    mov     esi, [ebp + 12]    ; esi = src
    mov     ecx, [ebp + 16]    ; ecx = n
    
    mov     eax, edi           ; Sauvegarde dest pour le retour
    cmp     ecx, 0             ; Si n == 0
    je      .end               ; Quitter directement
    
.loop:
    mov     bl, [esi]          ; Lire 1 octet depuis src
    mov     [edi], bl          ; Écrire dans dest
    inc     esi                ; src++
    inc     edi                ; dest++
    dec     ecx                ; n--
    jnz     .loop              ; Continuer si ecx != 0
    
.end:
    pop     edi                ; Restaure edi
    pop     esi                ; Restaure esi
    pop     ebp                ; Restaure ebp
    ret                        ; Retourne (eax contient dest)
```

### 📖 Explication détaillée

#### Setup du stack frame
```asm
push    ebp
mov     ebp, esp
push    esi
push    edi
```
- Sauvegarde de `ebp` pour restaurer l'état précédent
- `ebp` devient le nouveau point de référence pour accéder aux paramètres
- Sauvegarde de `esi` et `edi` car nous allons les modifier

#### Récupération des paramètres
```asm
mov     edi, [ebp + 8]     ; Premier paramètre : dest
mov     esi, [ebp + 12]    ; Deuxième paramètre : src
mov     ecx, [ebp + 16]    ; Troisième paramètre : n
```

**Organisation de la pile :**
```
[ebp + 16]  →  n (size_t)
[ebp + 12]  →  src (const void*)
[ebp + 8]   →  dest (void*)
[ebp + 4]   →  Adresse de retour
[ebp]       →  Ancien ebp
```

#### Préparation du retour
```asm
mov     eax, edi           ; Sauvegarde dest
cmp     ecx, 0             ; Vérifie si n == 0
je      .end               ; Si oui, termine
```
`eax` contiendra la valeur de retour (convention x86 : retour dans `eax`).

#### Boucle de copie
```asm
.loop:
    mov     bl, [esi]      ; Charge 1 octet de [esi] dans bl
    mov     [edi], bl      ; Écrit bl dans [edi]
    inc     esi            ; src++
    inc     edi            ; dest++
    dec     ecx            ; n--
    jnz     .loop          ; Si ecx != 0, continue
```

Équivalent C :
```c
while (n > 0) {
    *dest = *src;
    dest++;
    src++;
    n--;
}
```

#### Nettoyage et retour
```asm
.end:
    pop     edi
    pop     esi
    pop     ebp
    ret
```
Restaure l'état des registres et retourne (la valeur dans `eax` est automatiquement retournée).

### 💡 Utilisation
Cette fonction est utilisée dans `terminal_scroll()` pour copier efficacement les lignes VGA :
```c
ft_memcpy((void*)terminal_buffer, 
          (void*)(terminal_buffer + VGA_WIDTH), 
          (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(u16));
```

---

## 3. ft_memset

### 📌 Prototype
```c
void *ft_memset(void *s, int c, size_t n);
```

### 🎯 Fonction
Remplit une zone mémoire avec un octet spécifique. Écrit `n` fois la valeur `c` à l'adresse `s`. Retourne `s`.

### 🔍 Code complet
```asm
section .text
    global  ft_memset

ft_memset:
    push    ebp                ; Sauvegarde le base pointer
    mov     ebp, esp           ; Établit le stack frame
    push    edi                ; Sauvegarde edi
    
    mov     edi, [ebp + 8]     ; edi = s (destination)
    mov     eax, [ebp + 12]    ; eax = c (valeur à écrire)
    mov     ecx, [ebp + 16]    ; ecx = n (nombre d'octets)
    
    mov     edx, edi           ; Sauvegarde s pour le retour
    cmp     ecx, 0             ; Si n == 0
    je      .end               ; Quitter directement
    
.loop:
    mov     [edi], al          ; Écrire l'octet c dans [edi]
    inc     edi                ; s++
    dec     ecx                ; n--
    jnz     .loop              ; Continuer si ecx != 0
    
.end:
    mov     eax, edx           ; Restaure s pour le retour
    pop     edi                ; Restaure edi
    pop     ebp                ; Restaure ebp
    ret                        ; Retourne (eax contient s)
```

### 📖 Explication détaillée

#### Setup du stack frame
```asm
push    ebp
mov     ebp, esp
push    edi
```
- Sauvegarde de `ebp` pour restaurer l'état précédent
- `ebp` devient le nouveau point de référence pour accéder aux paramètres
- Sauvegarde de `edi` car nous allons le modifier

#### Récupération des paramètres
```asm
mov     edi, [ebp + 8]     ; Premier paramètre : s (void*)
mov     eax, [ebp + 12]    ; Deuxième paramètre : c (int)
mov     ecx, [ebp + 16]    ; Troisième paramètre : n (size_t)
```

**Organisation de la pile :**
```
[ebp + 16]  →  n (size_t)
[ebp + 12]  →  c (int)
[ebp + 8]   →  s (void*)
[ebp + 4]   →  Adresse de retour
[ebp]       →  Ancien ebp
```

#### Préparation du retour
```asm
mov     edx, edi           ; Sauvegarde l'adresse s dans edx
cmp     ecx, 0             ; Vérifie si n == 0
je      .end               ; Si oui, termine
```
`edx` conserve la valeur initiale de `s` car `edi` sera incrémenté dans la boucle. La valeur de retour sera dans `eax` à la fin.

#### Boucle de remplissage
```asm
.loop:
    mov     [edi], al      ; Écrit l'octet al dans [edi]
    inc     edi            ; s++
    dec     ecx            ; n--
    jnz     .loop          ; Si ecx != 0, continue
```

**Note importante :** On utilise `al` (les 8 bits de poids faible de `eax`) car on veut écrire un seul octet, même si `c` est passé comme un `int`.

Équivalent C :
```c
while (n > 0) {
    *s = (unsigned char)c;
    s++;
    n--;
}
```

#### Nettoyage et retour
```asm
.end:
    mov     eax, edx       ; Place l'adresse originale dans eax
    pop     edi
    pop     ebp
    ret
```
Restaure l'état des registres et retourne le pointeur `s` original (stocké dans `edx`, puis transféré dans `eax`).

### 💡 Utilisation
Cette fonction est typiquement utilisée pour initialiser des zones mémoire, par exemple pour effacer un buffer :
```c
ft_memset(terminal_buffer, 0, VGA_WIDTH * VGA_HEIGHT * sizeof(u16));
```
Ou pour initialiser des structures :
```c
struct my_struct data;
ft_memset(&data, 0, sizeof(data));  // Mise à zéro de la structure
```

---

## 4. ft_strlen

### 📌 Prototype
```c
size_t ft_strlen(const char *s);
```

### 🎯 Fonction
Calcule la longueur d'une chaîne de caractères (nombre de caractères avant `\0`).

### 🔍 Code complet
```asm
global  ft_strlen

ft_strlen:
    push    ebp                ; Sauvegarde le base pointer
    mov     ebp, esp           ; Établit le stack frame
    mov     eax, 0             ; Compteur = 0
    mov     edi, [ebp + 8]     ; edi = pointeur sur la chaîne
    
.loop:
    cmp     byte [edi + eax], 0  ; Compare avec '\0'
    je      .end                 ; Si '\0', termine
    inc     eax                  ; Compteur++
    jmp     .loop                ; Continue
    
.end:
    pop     ebp                ; Restaure ebp
    ret                        ; Retourne (eax contient la longueur)
```

### 📖 Explication détaillée

#### Setup et initialisation
```asm
push    ebp
mov     ebp, esp
mov     eax, 0             ; Le compteur commence à 0
mov     edi, [ebp + 8]     ; edi = paramètre 's'
```

`eax` servira de compteur et contiendra la valeur de retour.

#### Boucle de comptage
```asm
.loop:
    cmp     byte [edi + eax], 0  ; Compare l'octet à l'index eax avec 0
    je      .end                 ; Si c'est '\0', on a fini
    inc     eax                  ; Sinon, on incrémente le compteur
    jmp     .loop                ; Et on continue
```

**Détail de `byte [edi + eax]` :**
- `edi` contient l'adresse de base de la chaîne
- `eax` est l'index courant
- `byte` indique qu'on lit 1 octet
- On accède donc à `s[eax]`

Équivalent C :
```c
size_t len = 0;
while (s[len] != '\0') {
    len++;
}
return len;
```

#### Retour
```asm
.end:
    pop     ebp
    ret
```
La valeur dans `eax` (le compteur) est automatiquement retournée.

### 💡 Utilisation
Cette fonction est utilisée partout où on a besoin de connaître la longueur d'une chaîne :
```c
void terminal_write_string(const char *data)
{
    terminal_write(data, ft_strlen(data));
}
```

---

## 📚 Ressources

### Documentation officielle
- **OSDev – Bare Bones**  
  [https://wiki.osdev.org/Bare_Bones](https://wiki.osdev.org/Bare_Bones)

- **OSDev – Inline Assembly**  
  [https://wiki.osdev.org/Inline_Assembly/Examples](https://wiki.osdev.org/Inline_Assembly/Examples)

- **Multiboot Specification**  
  [https://www.gnu.org/software/grub/manual/multiboot/multiboot.html](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)

### Tutoriels assembleur x86
- **x86 Assembly Guide (Yale)**  
  [https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html](https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html)

- **Intel 80386 Programmer's Reference**  
  [https://pdos.csail.mit.edu/6.828/2018/readings/i386.pdf](https://pdos.csail.mit.edu/6.828/2018/readings/i386.pdf)

- **Felix Cloutier's x86 and amd64 Instruction Reference**  
  [https://www.felixcloutier.com/x86/](https://www.felixcloutier.com/x86/)  
  Référence complète et moderne de toutes les instructions x86/x86-64 avec explications détaillées

---

## 🎯 Points clés à retenir

### Conventions d'appel x86 (cdecl)
1. **Paramètres** : Passés sur la pile (de droite à gauche)
2. **Valeur de retour** : Dans le registre `eax`
3. **Registres à sauvegarder** : `ebx`, `esi`, `edi`, `ebp`
4. **Nettoyage de la pile** : Fait par l'appelant

### Organisation de la pile
```
[ebp + 16]  →  3ème paramètre
[ebp + 12]  →  2ème paramètre
[ebp + 8]   →  1er paramètre
[ebp + 4]   →  Adresse de retour
[ebp]       →  Ancien ebp (sauvegardé)
[ebp - 4]   →  Variables locales...
```

### Registres importants
- **EAX** : Valeur de retour, registre général
- **EBX, ECX, EDX** : Registres généraux
- **ESI** : Source index (source pour les opérations sur chaînes)
- **EDI** : Destination index (destination pour les opérations sur chaînes)
- **EBP** : Base pointer (référence pour les variables locales)
- **ESP** : Stack pointer (sommet de la pile)

---

Ces fonctions constituent la fondation bas-niveau du kernel KFS-1, permettant :
- ✅ Le boot et l'initialisation du système
- ✅ Les opérations mémoire optimisées
- ✅ Les utilitaires de manipulation de chaînes

Elles travaillent en synergie avec le code C pour créer un kernel fonctionnel.