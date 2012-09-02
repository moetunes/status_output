#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#define DEVICE "wlan0"
#define NETSPEEDFILE "/proc/net/dev"

static void update_speed();
static void update_speed2();

static char speed_ret[50];
static double tj;
static unsigned int recd, sent;

double time_so_far() {
   struct timeval tp;

   if (gettimeofday(&tp, (struct timezone *) NULL) == -1)
       perror("gettimeofday");
   return ((double) (tp.tv_sec)) +
     (((double) tp.tv_usec) * 0.000001 );
}

void update_speed() {
    FILE *f1;
    char line[256];

    f1 = fopen(NETSPEEDFILE, "r");
    if (f1 != NULL) {
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, " wlan0", 6) == 0)
                break;
        }
    } else {
        sprintf(speed_ret, "NET FAIL F");
        return;
    }
    fclose(f1);

    char *vals;
    unsigned int last_recd, last_trans;
    unsigned int down, up;
    double delta, down_speed, up_speed, current;

    /* get delta */
    current = time_so_far();
    delta = current - tj;
    if (delta <= 0.0001) {
        sprintf(speed_ret, "NET FAIL T");
        return;
    }
    tj = current;

    vals = strchr(line, ':');
    ++vals;

    last_recd = recd;
    //last_trans = sent;

    /* bytes packets errs drop fifo frame compressed multicast|bytes ... */
    sscanf(vals, "%d %*d %*d %*d %*d %*d %*d %*d %d",
        &down, &up);

    /* if recd or trans is less than last time, an overflow happened */
    if (down < last_recd) {
        last_recd = 0;
    } else {
        recd = down;
    }

    if (up < last_trans) {
        last_trans = 0;
    } else {
        sent = up;
    }

    /* calculate speeds */
    down_speed = (recd - last_recd) / delta;
    //up_speed = (sent - last_trans) / delta;
    if(down_speed > 1000000)
        sprintf(speed_ret, "%.2f MB/s %.2f MB/s", down_speed/1000000, up_speed/1000000);
    else if(down_speed > 1000)
        sprintf(speed_ret, " %.1f KB/s %.1f KB/s", down_speed/1000, up_speed/1000);
    else
        sprintf(speed_ret, " %.0f B/s %.0f B/s", down_speed, up_speed);
}

void update_speed2() {
    FILE *f1;
    char line[256];

    f1 = fopen(NETSPEEDFILE, "rb");
    if (f1 != NULL) {
        while(fgets(line, sizeof line, f1) != NULL) {
            // check NETSPEEDFILE for spaces before eth0/wlan0
            if (strncmp(line, " wlan0", 6) == 0)
                break;
        }
    } else {
        sprintf(speed_ret, "NET FAIL F");
        return;
    }
    fclose(f1);

    char *vals;
    unsigned int last_recd;
    int down;
    double delta, down_speed, current;

    /* get delta */
    current = time_so_far();
    delta = current - tj;
    if (delta <= 0.0001) {
        sprintf(speed_ret, "NET FAIL T");
        return;
    }
    tj = current;

    vals = strchr(line, ':');
    ++vals;

    last_recd = recd;

    /* bytes packets errs drop fifo frame compressed multicast|bytes ... */
    sscanf(vals, "%d %*d %*d %*d %*d %*d %*d %*d %*d",
        &down);

    /* if recd is less than last time, an overflow happened */
    if (down < last_recd) {
        last_recd = 0;
    } else {
        recd = down;
    }

    /* calculate speeds */
    down_speed = (recd - last_recd) / delta;
    //printf("DOWN= %d, lr= %d, nsr= %d\n", down, last_recd, recd);
    if(down_speed > 1000000.0)
        sprintf(speed_ret, "%.2f MB/s", down_speed/1000000.0);
    else if(down_speed > 1000)
        sprintf(speed_ret, " %.1f KB/s", down_speed/1000.0);
    else
        sprintf(speed_ret, " %.0f B/s", down_speed);

    return;
}

int main() {
    while (1) {
        update_speed2();
        printf("            \r** %s", speed_ret);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}