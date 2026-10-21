.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM non defini. Utilise l'image devkitpro/devkitarm")
endif

include $(DEVKITARM)/3ds_rules

TARGET          := heartfight
BUILD           := build
SOURCES         := source
APP_TITLE       := Coeur Fragile
APP_DESCRIPTION := Jeu de boss facon bullet hell
APP_AUTHOR      := Moi
ICON            := assets/icon.png

CTRULIB := $(DEVKITPRO)/libctru
ARCH    := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS  := -g -Wall -O2 -mword-relocations -ffunction-sections -fomit-frame-pointer \
           $(ARCH) -std=gnu11 -D__3DS__ -I$(CTRULIB)/include
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(BUILD)/$(TARGET).map -L$(CTRULIB)/lib
LIBS    := -lctru -lm

CFILES  := $(wildcard $(SOURCES)/*.c)
OFILES  := $(patsubst $(SOURCES)/%.c,$(BUILD)/%.o,$(CFILES))

.PHONY: all cia clean

all: $(TARGET).3dsx

$(BUILD)/%.o: $(SOURCES)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OFILES)
	$(CC) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

$(TARGET).smdh: $(ICON)
	smdhtool --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(ICON) $@

$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	3dsxtool $< $@ --smdh=$(TARGET).smdh

cia: $(TARGET).3dsx
	bash tools/make_cia.sh $(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).3dsx $(TARGET).smdh $(TARGET).cia banner.bnr icon.icn
