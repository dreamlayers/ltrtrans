CC=i686-w64-mingw32-gcc -mwindows
CFLAGS=-Wall -g

.PHONY: all
all: hookdll.dll hookexe.exe

hookdll.dll: hookdll.c hookdll.def
	$(CC) $(CFLAGS) -mdll -Wl,--enable-stdcall-fixup -Wno-unknown-pragmas -o $@ $< hookdll.def

hookres.o: hookexe.rc hookexe.ico small.ico
	windres $< -o $@

hookexe.exe: hookexe.c hookres.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f hookdll.dll hookres.o hookexe.exe
