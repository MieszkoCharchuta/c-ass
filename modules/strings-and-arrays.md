# Strings, arrays, and pointers

We already went through [basics on strings and string
literals](/modules/strings.md).

So, given that a string constant is internally used to initialize an array of
`char`s, and an array name represents its first element memory address, the
value of a string is an adress of its first character.

:eyes: [string-const-address.c](/src/string-const-address.c)

You can use an array notation with pointers.  So, if you **really** wanted, you
could do something like this:

```C
printf("%c\n", "hello, world"[1]);	// will print `e`
```

## Strings and pointers

Given that a string constant is a pointer to its first character, we can use it
like this:

```
char *p = "hello";

printf("%c\n", p[1]);   // will print 'e'
```

:eyes: [array-notation-with-ptr.c](/src/array-notation-with-ptr.c)

:heavy\_exclamation\_mark: Pointer initialized with a string literal **may not**
be changed in the same way as array.  Whether the internal array created and
initialized from the string literal lays in read-write or read-only memory is
implementation defined.  For `gcc` and `clang`, it is read-only memory.

:eyes: [string-literal-write.c](/src/string-literal-write.c)

And as you can use a pointer notation with arrays (we already mentioned that the
array name is a synonym for the address of its first element), you can do this:

```
char a[] = "hello, world";

printf("%c\n", *(a + 1));       // will print 'e'
```

[ptr-with-array-notation.c](/src/ptr-with-array-notation.c)