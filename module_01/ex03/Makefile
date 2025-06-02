# ----------------  COLORS  ------------------------------------------------- #
RED				=	\\033[0;31m
ORANGE			=	\033[0;38;5;208m
GREEN	    	=	\033[1;32m
DEFAULT			=	\\033[0m

# ----------------  FILES  -------------------------------------------------- #
SRC				=	main.c
BIN				=	main.bin
HEX				=	main.hex

# ----------------  MICROCONTROLLER  ---------------------------------------- #
MCU				=	atmega328p
F_CPU			=	16000000UL
BAUD			=	115200
PROGRAMMER		=	arduino
PORT			=	/dev/ttyUSB0

# ----------------  MAC CONFIG  --------------------------------------------- #
ifeq ($(shell uname), Darwin)
    PORT := /dev/cu.usbserial-10
endif

# ----------------  FLAGS  -------------------------------------------------- #
CFLAGS			=	-mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os

# ----------------  COMMANDS  ----------------------------------------------- #
CC				=	avr-gcc
OBJCOPY			=	avr-objcopy
AVRDUDE			=	avrdude

# ----------------  RULES  -------------------------------------------------- #
all:				hex flash

hex:				$(SRC)
					$(CC) $(CFLAGS) -o $(BIN) $(SRC)
					$(OBJCOPY) -O ihex $(BIN) $(HEX)
					@echo "$(GREEN)$(HEX) generated from $(BIN)$(DEFAULT)"

flash:				hex
					$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$(HEX):i
					@echo "$(GREEN)$(HEX) copied into microcontroller flash memory$(DEFAULT)"

clean:
					rm $(HEX) $(BIN)
					@echo "$(GREEN)Cleaned $(HEX) & $(BIN)$(DEFAULT)"

.PHONY: 			all hex flash clean