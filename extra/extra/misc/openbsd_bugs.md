The majority of SCN* macros differs from their PRI* cousins. When you define unsigned int you should always follow the C standards that made it clear what format specifier unsigned int should use, unfortunately the majority of OpenBSD's own libc SCN* macros managed not to follow the standard. "hu" and "hhu" are not unsigned int format specifiers, tried to get in touch with OpenBSD devs, reported this bug but no one responded.

If they wanted to use specific integer type, then they should define it as such earlier. 8 bit integer is the smallest integer type in existance and for sure it cannot represent the 32 bit UINT_MAX number. glibc uses the maximum unsigned char and signed char for all int8_t, int_least8_t, int_fast8_t and uint8_t, uint_least8_t, uint_fast8_t , and yes glibc defines corrent PRI* and SCN* format specifier macros to match that integer type.

It's not like to mistake the float format specifier with the "double" one.

It's dirty trick to let the complier do the conversion for you to lower the integer type and not follow the standards to define it as such earlier.

The definitions in OpenBSD own libc:

```cpp
/* machine/_types.h */
typedef	unsigned int		__uint32_t;

typedef	__uint32_t		__uint_fast8_t;
typedef	__uint32_t		__uint_fast16_t;

/* stdint.h */
typedef	__uint_fast8_t		uint_fast8_t;
typedef	__uint_fast16_t		uint_fast16_t;

/* inttypes.h */
#define	PRIuFAST8		"u"		/* uint_fast8_t */
#define	SCNuFAST8		"hhu"		/* uint_fast8_t */

#define	PRIuFAST16		"u"		/* uint_fast16_t */
#define	SCNuFAST16		"hu"		/* uint_fast16_t */
```

Sample program to simulate the bug:

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main(void) {
  char acc[] = "Remaining: 99000";
  uint_fast8_t cur = 0, started = 99000;

  printf("%s\n", acc);
  if (EOF == (sscanf(acc, "%*s %"SCNuFAST8, &cur))) {
    return EXIT_FAILURE;
  }
  printf("%s " "%"PRIuFAST8 " %s " "%"PRIuFAST8 "\n",
   "Started with: $", started,
   "The current balance now: $", cur
  );

  return EXIT_SUCCESS;
}
```

Amusing representation of the above code:

```cpp
unsigned long long int small_int = 240;

printf("%hhu\n", small_int);
```
