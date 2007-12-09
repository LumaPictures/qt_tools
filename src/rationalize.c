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

    // limit maximum number of coefficients. They're cheap, but really
    // you can't use more than *maybe* a dozen before the denominator
    // is crazy-big.

    #define N 30
    long a[N];

    // we only work on positive inputs
    if(n < 0)
        n = -n;
    
    int i = 0; // go until N or no fraction left

    //printf("[%f] \n",n);
    while(n != 0 && i < N)
    {
        double w = floor(n);
        //printf("%d. <%f,%f> \n",i,n,w);
        a[i++] = w;
        n -= w;
        if ( n < (0.5 / (double)denominatorMax) )
            n = 0;
        //printf("n is %f, n==0 = %d\n",n,n==0);
        if(n != 0)
            n = 1/n;
        //printf("n is %f\n",n);
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
    if(argc == 2)
    {
        double n;
        sscanf(argv[1],"%lf",&n);
        tryRationalize(n);
        return 0;
    }

    if(argc == 3)
    {
        double num;
        double den;
        sscanf(argv[1],"%lf",&num);
        sscanf(argv[2],"%lf",&den);
        tryRationalize(num/den);
        return 0;
    }

    if(argc == 5)
    {
        double num;
        double den;
        double maxNum;
        double maxDen;
        unsigned long long rNum;
        unsigned long long rDen;
        sscanf(argv[1],"%lf",&num);
        sscanf(argv[2],"%lf",&den);
        sscanf(argv[3],"%lf",&maxNum);
        sscanf(argv[4],"%lf",&maxDen);
        rationalize(num/den,maxNum,maxDen,&rNum,&rDen);

        printf("%f --> %lld/%lld (%f)\n",num/den,rNum,rDen,(double)rNum/(double)rDen);

        return 0;
    }

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

    tryRationalize(1000000000);
}

#endif // RATIONALIZE_TESTS

// The End
