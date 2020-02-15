CC = gcc
Target = ppkage.exe
Object = \
			ppkg_main.o \
			ppkg_com.o

$(Target): $(Object)
	gcc $(Object) -o $(Target)

$(Object): %.o: %.c
	$(CC) -c -Wall -fexec-charset=gbk $< -o $@

clean:
	del *.exe *.o
