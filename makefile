WARNS=-Wall -ansi -pedantic -std=c89
LEVEL=-O0
FRAMEWORK=-F/Library/Frameworks -framework
ENTRY=main.c

all:
	gcc $(ENTRY) $(LEVEL) $(WARNS) $(FRAMEWORK) SDL2 -o prog

run:
	gcc $(ENTRY) $(LEVEL) $(WARNS) $(FRAMEWORK) SDL2 -o prog && ./prog

undef:
	gcc $(ENTRY) -O0 -fsanitize=undefined $(WARNS) $(FRAMEWORK) SDL2 -o prog && ./prog

debug:
	gcc $(ENTRY) -O0 $(WARNS) $(FRAMEWORK) SDL2 -g -o prog && lldb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	
