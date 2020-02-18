CC = gcc
Target = ppkage.exe
Object = \
			ppkg_main.o \
			ppkg_com.o \
			ppkg_generator.o \
			debug.o

$(Target): $(Object)
	gcc -g $(Object) -o $(Target)

$(Object): %.o: %.c
	$(CC) -c -Wall -g -fexec-charset=gbk $< -o $@

clean:
	del *.exe *.o
