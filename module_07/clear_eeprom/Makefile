# ----------------  COLORS  ------------------------------------------------- #
RED				=	\\033[0;31m
ORANGE			=	\033[0;38;5;208m
GREEN	    	=	\033[1;32m
DEFAULT			=	\\033[0m

# ----------------  FILES  -------------------------------------------------- #
SRC				=	main.c
BIN				=	main.bin
HEX				=	main.hex
OBJ				=	$(SRC:.c=.o)

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
RM				=	rm -f

# ----------------  RULES  -------------------------------------------------- #
all:				hex flash

screen:				hex flash
					screen $(PORT) $(BAUD)

hex:				$(HEX)

$(HEX):				$(BIN)
					$(OBJCOPY) -O ihex $(BIN) $(HEX)
					@echo "$(GREEN)$(HEX) generated from $(BIN)$(DEFAULT)"

$(BIN):				$(OBJ)
					$(CC) $(CFLAGS) -o $(BIN) $(OBJ)
					@echo "$(GREEN)$(BIN) generated$(DEFAULT)"

$(OBJ):				$(SRC)
					$(CC) $(CFLAGS) -c $(SRC)

flash:				$(HEX)
					$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$(HEX):i
					@echo "$(GREEN)$(HEX) copied into microcontroller flash memory$(DEFAULT)"

clean:
					$(RM) $(HEX) $(BIN) $(OBJ)
					@echo "$(GREEN)Cleaned $(HEX) & $(BIN)$(DEFAULT)"

.PHONY: 			all hex flash clean