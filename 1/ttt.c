#include <stdio.h>
#include <sys/time.h>

int main() {

    struct timeval start, end;

    // começo da mediçao
    gettimeofday(&start, NULL); 

    // carga de trabalho
    volatile double x = 0;
    for(x=0; x<10000000000; x++);

    // fim da mediçao
    gettimeofday(&end, NULL);

    int duration;

    duration = (end.tv_sec*1000 + end.tv_usec/1000) - (start.tv_sec*1000 + start.tv_usec/1000);

    printf("Tempo = %d ms\n", duration);

    return 0;



}