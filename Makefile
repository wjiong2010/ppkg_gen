CC = gcc
Target = ppkage.exe
CFlags	= -g -c -Wall -fexec-charset=gbk
Object = \
			ppkg_main.o \
			ppkg_com.o \
			ppkg_generator.o \
			version.o \
			debug.o

#debug: CFlags += -g
#debug:$(Target)

PPKG_VER ?= A01V01

APP_OPTIONS += -DPPKG_VER_STR=\"$(PPKG_VER)\"

$(Target): $(Object)
	@echo $(Object)
	@echo $(Target)
	$(CC) $(Object) -o $(Target)

$(Object): %.o: %.c
	$(CC) $(CFlags) $(APP_OPTIONS) $< -o $@

clean:
	del *.exe *.o
