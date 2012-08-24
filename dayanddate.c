#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void get_day_date();

static char daydate_ret[7];

void get_day_date() {
    struct tm tm; 
    time_t    time_value = time(0);
    tm = *localtime(&time_value);

    switch(tm.tm_wday) {
        case 0:
            sprintf(daydate_ret, "Sun %d", tm.tm_mday);
            break;
        case 1:
            sprintf(daydate_ret, "Mon %d", tm.tm_mday);
            break;
        case 2:
            sprintf(daydate_ret, "Tue %d", tm.tm_mday);
            break;
        case 3:
            sprintf(daydate_ret, "Wed %d", tm.tm_mday);
            break;
        case 4:
            sprintf(daydate_ret, "Thu %d", tm.tm_mday);
            break;
        case 5:
            sprintf(daydate_ret, "Fri %d", tm.tm_mday);
            break;
        case 6:
            sprintf(daydate_ret, "Sat %d", tm.tm_mday);
            break;
    }
    
    return;
}

int main() {
    get_day_date();
    printf("TODAY %s\n", daydate_ret);
    return 0;
}
