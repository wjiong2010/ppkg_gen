CC = gcc
Target = ppkage.exe
CFlags	= -g -c -Wall -fexec-charset=gbk
Object = \
			ppkg_main.o \
			ppkg_com.o \
			ppkg_generator.o \
			debug.o

#debug: CFlags += -g
#debug:$(Target)
 
$(Target): $(Object)
	$(CC) $(Object) -o $(Target)

$(Object): %.o: %.c
	$(CC) $(CFlags) $< -o $@

clean:
	del *.exe *.o
