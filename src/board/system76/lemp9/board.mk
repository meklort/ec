EC=it5570e

# Add keymap to src
KEYMAP?=default
SRC+=$(BOARD_DIR)/keymap/$(KEYMAP).c

# Set battery I2C bus
CFLAGS+=-DI2C_SMBUS=I2C_4

# Set touchpad PS2 bus
CFLAGS+=-DPS2_TOUCHPAD=PS2_3

# Set smart charger parameters
# Input sense resistor requires a multiplier of 2 to achieve a 1:1 mapping
CFLAGS+=\
	-DCHARGER_CHARGE_CURRENT=3314 \
	-DCHARGER_CHARGE_VOLTAGE=8800 \
	-DCHARGER_INPUT_CURRENT=3200 \
	-DCHARGER_INPUT_MULT=2

# Add system76 common code
include src/board/system76/common/common.mk
