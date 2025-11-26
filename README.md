# Arduino-Dos
A Linux Operating System for any Arduino Microcontroller board
It features a **Bash-like shell**, a **login system**, simulated files, and a collection of basic commands.
## Features

- Bash-like command prompt `[user@avr-dos /]$` or `[root@avr-dos /]$`
# How To Compile
Run These Commands:
in Linux:
avr-gcc -mmcu=atmega2560 -DF_CPU=16000000UL -Os -Wall main.c -o arduinoDOS.elf
avr-objcopy -O ihex -R .eeprom arduinoDOS.elf arduinoDOS.hex
