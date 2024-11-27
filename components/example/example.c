#include <stdio.h>

#include "example.h"

void example_func(void) {
    printf("This is Example Component.\r\n");

#ifdef CONFIG_EXAMPLE_COMPONENT_FLAG
    printf("Example component secret message\r\n");
#endif
}
