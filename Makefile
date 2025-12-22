ASM = nasm
CC = gcc
LD = ld

ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -ffreestanding -Wall -Wextra -Werror -c
LDFLAGS = -m elf_i386 -T $(SRC_DIR)/linker.ld

SRC_DIR = srcs
BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)

ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

KERNEL = $(BUILD_DIR)/kernel.bin
ISO = kfs-1.iso

all: $(ISO)

$(ISO): $(KERNEL)
	@mkdir -p $(GRUB_DIR)
	@cp $(KERNEL) $(BOOT_DIR)/kernel.bin
	@echo '' >> $(GRUB_DIR)/grub.cfg
	@echo 'menuentry "KFS-1" {' >> $(GRUB_DIR)/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(GRUB_DIR)/grub.cfg
	@echo '    boot' >> $(GRUB_DIR)/grub.cfg
	@echo '}' >> $(GRUB_DIR)/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null || grub2-mkrescue -o $(ISO) $(ISO_DIR)
	@echo -e "\033[32mISO créée : $(ISO)\033[0m"

$(KERNEL): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	@$(LD) $(LDFLAGS) -o $@ $^
	@echo -e "\033[32mKernel compilé : $(KERNEL)\033[0m"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	@$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) $< -o $@

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

fclean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)
	@echo -e "\033[31mFiles and folder : $(BUILD_DIR) $(ISO_DIR) $(ISO) deleted\033[0m"

re: fclean all

.PHONY: all fclean re run
