## System call tracing

- system calls: interface to the kernel to provide services like memory
  allocation, I/O, ...
- prints system call arguments, return values, timing information etc.
- this works by inserting break points
  - the break points make the program slower, this overhead could be much larger
    than what is allowed in production

- basic tools:
  - `strace`/`truss`/`ltrace`
  - `dtrace` (dynamic tracing)

:wrench: Task: write a program that opens a file specified by the first argument
  of the program and reads number of bytes specified by the second argument and
  writes them to standard output. Use fopen()/fread() without checking return
  values or error conditions.  What happens if the program tries to read files
  like `/etc/shadow` on Unix ?  Run the program under strace to see possible
  clues. Refactor the program so that the I/O is performed in a function:

```C
        static void file_read(char *file, size_t len);
```
 so that the function is performed in a cycle with 16k iterations and run
 the program with `/etc/passwd`.

 Again run the program under strace to see possible clues.

 Now fix the program so that:
   - it properly detects all errors
   - the function can be called arbitrary number of times (in sequence)

#source file-open.c