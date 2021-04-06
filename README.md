# Uxn

A [stack-based VM](https://wiki.xxiivv.com/site/uxn.html), written in ANSI C. 

## Build

To build the Uxn emulator, you must have [SDL2](https://wiki.libsdl.org/).

```sh
./build.sh 
	--debug # Add debug flags to compiler
	--cli # Run rom without graphics
```

## Emulator Controls

- `ctrl+h` toggle debugger
- `alt+h` toggle zoom

## Uxambly

Read more in the [Uxambly Guide](https://wiki.xxiivv.com/site/uxambly.html).

```
%RTN { JMP2r }

( devices )

|0110 ;Console { vector 2 pad 6 char 1 byte 1 short 2 }

( program )

|0200
	
	,text1 ,print-label JSR2
	,text2 ,print-label JSR2
	#ab =Console.byte
	#cdef =Console.short

BRK

@print-label ( text )
	
	$loop
		( send ) DUP2 PEK2 =Console.char
		( incr ) #0001 ADD2
		( loop ) DUP2 PEK2 #00 NEQ ^$loop JNZ
	POP2

RTN    

@text1 [ Welcome 20 to 20 UxnVM 0a00 ]
@text2 [ Hello 20 World 0a00 ] 
```

## TODOs

- Shortcut to export/import disk state
- Implement Uxambly REPL
- Load disks at a different place than 0x0000.

## Palettes

- `[ 6a03 4a0d aa0c ]`, purple/cyan
- `[ a1f3 a14d a16c ]`, grey-pink/teal

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
