#include <stdio.h>

int roundUp(unsigned int x) {
    if (x <= 16)
        return 16;
    else {
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x++;
        return x;
    }
}

int main() {
    printf("%d\n", roundUp(4));
    printf("%d\n", roundUp(16));
    printf("%d\n", roundUp(17));
    printf("%d\n", roundUp(48));
    printf("%d\n", roundUp(64));
}