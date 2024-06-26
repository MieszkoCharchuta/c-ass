# Valid pointer operations

Valid pointer operations are:

- Assignment of pointers of the same type.
- Adding and subtracting a pointer and an integer.
- Subtracting and comparing two pointers to members of the same array.
- Assigning and comparing a pointer to zero.

All other pointer arithmetic are invalid operations and may or may not trigger a
warning.

More specifically, assigning an integer to a pointer is not a valid operation,
usually you get a warning.

```C
int
main(void)
{
	int *p = 0x1234;        // is not a valid behavior
}
```

```
$ cc int-to-ptr.c
int-to-ptr.c:4:7: warning: incompatible integer to pointer conversion initializing 'int *' with an
      expression of type 'int' [-Wint-conversion]
        int *p = 0x1234;        // is not a valid behavior
             ^   ~~~~~~
1 warning generated.
```

#source int-to-ptr.c
