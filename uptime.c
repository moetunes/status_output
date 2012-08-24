/* Build with
 * gcc uptime.c -o uptime_info -lprocps
 * /

#include <stdlib.h>
#include <stdio.h>
#include <proc/sysinfo.h>

static void get_uptime();

static char uptime_ret[15];

void get_uptime() {
    double up, idle;
    unsigned int days, hours, mins, nup;

    uptime(&up, &idle);
    nup = up;
    days = nup/86400;
    hours = (nup%86400)/3600;
    mins = (nup%3600)/60;
    if(days > 0) sprintf(uptime_ret, "%dd %dh %dm %ds", days,hours,mins,nup%60);
    else if(hours > 0) sprintf(uptime_ret, "%dh %dm %ds", hours,mins,nup%60);
    else if(mins > 0) sprintf(uptime_ret, "%dm %ds", mins,nup%60);
}

int main() {
    get_uptime();
    printf("%s\n", uptime_ret);
    return 0;
}
