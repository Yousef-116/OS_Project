
#include <inc/lib.h>


int fibonacci(int n);

void
_main(void)
{
	int i1=0;
	char buff1[256];
	atomic_readline("Please enter Fibonacci index:", buff1);
	i1 = strtol(buff1, NULL, 10);

	int res = fibonacci(i1) ;

	atomic_cprintf("Fibonacci #%d = %d\n",i1, res);
	return;
}


int fibonacci(int n)
{
	if (n <= 1)
		return 1 ;
	return fibonacci(n-1) + fibonacci(n-2) ;
}

