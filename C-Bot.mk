SHELL=cmd
CC = xc32-gcc
OBJCPY = xc32-bin2hex
ARCH = -mprocessor=32MX130F064B
PORTN=$(shell type COMPORT.inc)

OBJ = C-Bot.o

C-Bot.elf: $(OBJ)
	$(CC) $(ARCH) -o C-Bot.elf C-Bot.o -mips16 -DXPRJ_default=default -legacy-libc -Wl,-Map=C-Bot.map
	$(OBJCPY) C-Bot.elf
	@echo Success!
   
C-Bot.o: C-Bot.c
	$(CC) -g -x c -mips16 -Os -c $(ARCH) -MMD -o C-Bot.o C-Bot.c -DXPRJ_default=default -legacy-libc

clean:
	@del *.o *.d *.map *.elf *.hex 2>NUL

FlashLoad:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	pro32 -p -v C-Bot.hex
	putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N -v

putty:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N -v

dummy: C-Bot.hex C-Bot.map
	@echo ;-)
	
explorer:
	explorer .
