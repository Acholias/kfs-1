
# ------------------------------------------------------------
#  Makefile pour KFS-1
#  S'utilise après avoir lancé : nix develop
# ------------------------------------------------------------

ASM = nasm
CC = gcc
LD = ld

ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -m elf_i386 -T $(SRC_DIR)/linker.ld

SRC_DIR = srcs
BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

ASM_SOURCES = $(wildcard $(SRC_DIR)/*.asm)
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)

ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

KERNEL = $(BUILD_DIR)/kernel.bin
ISO = kfs-1.iso

# ------------------------------------------------------------
#  Règle principale
# ------------------------------------------------------------
all: $(ISO)

# ------------------------------------------------------------
#  Création de l'ISO bootable
# ------------------------------------------------------------
$(ISO): $(KERNEL)
	@mkdir -p $(GRUB_DIR)
	@cp $(KERNEL) $(BOOT_DIR)/kernel.bin

	@echo 'set timeout=0' > $(GRUB_DIR)/grub.cfg
	@echo 'set default=0' >> $(GRUB_DIR)/grub.cfg
	@echo '' >> $(GRUB_DIR)/grub.cfg
	@echo 'menuentry "KFS-1" {' >> $(GRUB_DIR)/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(GRUB_DIR)/grub.cfg
	@echo '    boot' >> $(GRUB_DIR)/grub.cfg
	@echo '}' >> $(GRUB_DIR)/grub.cfg

	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null \
	  || grub2-mkrescue -o $(ISO) $(ISO_DIR)

	@echo "ISO créée : $(ISO)"

# ------------------------------------------------------------
#  Link du kernel
# ------------------------------------------------------------
$(KERNEL): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel compilé : $(KERNEL)"

# ------------------------------------------------------------
#  Compilation ASM et C
# ------------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# ------------------------------------------------------------
#  Exécution avec QEMU
# ------------------------------------------------------------
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

# ------------------------------------------------------------
#  Nettoyage
# ------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)

re: clean all

.PHONY: all clean re run
