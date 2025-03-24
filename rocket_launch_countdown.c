#include <stdio.h>
#include <windows.h>
#include <math.h>

int main() {
    double countdown[] = {-10, -9, -8, -7, -6, -5, -4, -3, -2, -1, -0.0, +0.0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (int i = 0; i < sizeof(countdown) / sizeof(countdown[0]); i++) {
        Sleep(1000);

        if (signbit(countdown[i])) { 
            printf("T%g\n", countdown[i]);
            if (countdown[i] == -3.0) printf("Engine Start!\n"); 
            if (countdown[i] == -0.0) printf("Ignition!\n");
        } else {
            printf("T%+g\n", countdown[i]);
        }

        if (countdown[i] == 2.0) printf("Liftoff!\n");
    }

    return 0;
}
