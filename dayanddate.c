#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void get_day_date();

static char daydate_ret[7];

void get_day_date() {
    struct tm tm; 
    time_t    time_value = time(0);
    tm = *localtime(&time_value);

    strftime(daydate_ret, 7, "%a %d", &tm);
    
    return;
}

int main() {
    get_day_date();
    printf("TODAY %s\n", daydate_ret);
    return 0;
}
