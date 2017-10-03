#include <stdio.h>
#include <stdlib.h>
// #include <ucontext.h>

// #include "../include/cdata.h"
#include "../include/cthread.h"
// #include "../include/support.h"

void print1(void *pointer) {

	int x = *(int*)pointer;
	printf("%d\n", x);

	cyield();
	cyield();
	cyield();
	cyield();
	cyield();

	printf("%d\n", x*x );

	cyield();
	cyield();

	printf("termino primeira\n");
}

void print() {

	printf("%d\n", 99);

	cyield();
	cyield();
	cyield();
	cyield();

	cyield();
	cyield();
	cyield();


	printf("%d\n", 99999);


	cyield();
	cyield();
	cyield();	

	cyield();
	cyield();
	cyield();

	printf("termino sgunda\n");
}

int main(int argc, char const *argv[])
{
	int x = 12;
	int tidA = ccreate((void*)&print1, (void*)&x, 0);
	int tidB = ccreate((void*)&print, NULL, 0);
	printf("%d %d\n", tidA, tidB);

	cjoin(tidA);
	printf("primeira tem que ter termiando\n");
	
	cjoin(tidB);
	printf("segunda tem que ter termiando\n");

	cyield();//

	cyield();
	cyield();
	cyield();

	cyield();
	cyield();
	cyield();
	
	cyield();
	cyield();
	cyield();

	return 0;
}
