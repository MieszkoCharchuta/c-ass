#include <stdio.h>
#include <string.h>

static void
print_hex(char *ident, char *str, char *end)
{
	printf("%-10s", ident);
	for (char *p = str; p < end; p++)
		printf("%2hhx ", *p);
	printf("\n");
}

int
main(void)
{
	char *p;
	char str[] = "foo,,bar,fbar";
	char *end = str + strlen(str) + 1;
	char *inputstr = str;

	// Print the original string.
	print_hex("before: ", str, end);

	while ((p = strsep(&inputstr, ",")) != NULL)
		printf("'%s'\n", p);

	// Print the string now to see how it changed.
	print_hex("after: ", str, end);
}
