# nano-lang

A compiler and an assmbler for a extremely small language on Linux x86-64

# usage

```
% make
% echo 'puts "hello, world!"' | ./nanoc | ./nanoa > hello.o
% gcc hello.o -o hello
% ./hello
hello, world!
```

# files

```
nanoc.c : compiler
nanoa.c : assembler
```
