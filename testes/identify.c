#include <stdio.h>
#include <stdlib.h>

#include "../include/cthread.h"

int main(){
    char test[200];
    int num;
    num = 200;
    cidentify(test,num);
    printf("\nPrinting with %d characters:\n %s",num,test);

    return 0;
  }
