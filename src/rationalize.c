// file: rationalize.c
// 2006.08.20 david van brink
// subservientastronaut.com
//

#include <stdio.h>
#include <math.h>

// +------------------------
// | Take a number and approximate it as
// | a rational fraction.
// |
// | This uses "continued fractions", to
// | (more or less) keep propogating the error
// | along. Eventually some coefficients are
// | generated and used to construct the final
// | numerator and denominator returned.
// |
// | Use the "max" values to limit the
// | size of results to something reasonable.
// |

void rationalize(double n,
        unsigned long long numeratorMax,
        unsigned long long denominatorMax,
        unsigned long long *numeratorOut,
        unsigned long long *denominatorOut)
{
    // first: a very special case.
    if(n == 0)
    {
        *numeratorOut = 0;
        *denominatorOut = 1;
        return;
    }

    if(numeratorMax == 0)  
        numeratorMax = 1000000;
    if(denominatorMax == 0)
        denominatorMax = 1000000;

    // the coefficients must be kept few, for though
    // the large fractions do become frightfully accurate
    // in short order, the values do not fit in long longs!
    #define N 20
    long a[N];

    // we only work on positive inputs
    if(n < 0)
        n = -n;
    
    int i = 0; // go until N or no fraction left

    //printf("[%f] ",n);
    while(n < 999999 && n > .000001 && i < N)
    {
        double w = floor(n);
        //printf("<%f,%f> ",n,w);
        a[i++] = w;
        n -= w;
        n = 1/n;
    }

    // the coefficients of the iterative fraction are in place,
    // now compute the numerator & denominator...

    // |
    // | this loop walks "up" the list of coefficients and
    // | performs simple fraction-math to reduce them.
    // |
    // | For mathematical reasons I don't quite understand,
    // | it seems that numerator and denominator are always
    // | relatively prime, so no fraction-reduction is ever
    // | required.
    // |
    // | We use doubles in here to get the benefit of
    // | very large values and overflows.
    // |

    int iterations = i;
    int k;
reduce:
    k = iterations;
    if(k > iterations)
        k = iterations;

    // initial result is 1/0 = inf, since it gets reciprocal'd
    double numerator = 1; // numerator
    double denominator = 0; // denominator

    while(k > 0)
    {
        k--;
        double c = a[k];
        c *= numerator;
        c += denominator;

        denominator = numerator;
        numerator = c;
    }

    // Within our limits? if not, try again with fewer iterations.
    if(numerator > numeratorMax || denominator > denominatorMax)
        if(iterations > 1)
        {
            iterations--;
            goto reduce; // try again.
        }

    *denominatorOut = denominator;
    *numeratorOut = numerator;
}

#ifdef RATIONALIZE_TESTS

void tryRationalize(double n)
{
    unsigned long long num,den;
    rationalize(n,1000000,1000000,&num,&den);
    double n2 = (double)num / (double)den;
//    if(n != n2)
    printf("%.9f reduces to %llu/%llu (%.9f)\n",n,num,den,n2);

}

int main(int argc,char **argv)
{
    printf("x\n");

    tryRationalize(29.97);
    tryRationalize(.423423423423423);
    tryRationalize(12345.0/70909.0);
    tryRationalize(3.14159265358979323);
    double d;
    for(d = 0; d <= 10; d += .1)
        tryRationalize(d);

    int i;
    for(i = 0; i <= 70909; i++)
        tryRationalize((double)i / 70909.0);

    // verify pi works with different maximum fractionbits

    double pi = 3.14159265358979323846264338327950288;
    unsigned long long num,den;
    rationalize(pi,50,50,&num,&den);
    if(num == 22 && den == 7)
        printf("pi 22/7 ok\n");
    else
        printf("pi 22/7 ERROR\n");
    rationalize(pi,400,400,&num,&den);
    if(num == 355 && den == 113)
        printf("pi 355/113 ok\n");
    else
        printf("pi 355/113 ERROR\n");
}

#endif // RATIONALIZE_TESTS

// The End
