#include <stdio.h>

int main(int argc, void * arg[])
{
    while(1){
    printf("%s", ((char*) arg[0]));
    }

    return 0;
}