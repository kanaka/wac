#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

static const int n = 25;
static const int m = 75;

static const int maxiter = 100;

static const double rmin = -2.1;
static const double rmax = 0.5;
static const double imin = -1;
static const double imax = 1;

int main( ) {

    char *output = malloc(n * m);
    memset(output, ' ', n * m);

    // compute
    for(int i=0; i<n; i++) {
        double y = imin + (imax - imin) * i / (n - 1.0);
        for(int j=0; j<m; j++) {
            double x = rmin + (rmax - rmin) * j / (m - 1.0);

            double complex z = 0;
            double complex c = x + y * I;

            int it = 0;
            while((cabs(z) < 4) && (it < maxiter)) {
                ++it;
                z = z * z + c;
            }

            if(it >= maxiter) {
                output[i * m + j] = '*';
            }
        }
    }

    // output
    for(int i=0; i<n; i++) {
        for(int j=0; j<m; j++) {
            putc(output[i * m + j], stdout);
        }
        putc('\n', stdout);
    }


    free(output);

    return 0;
}
