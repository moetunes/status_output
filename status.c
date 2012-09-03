/* build with
 * gcc status.c -o status_output -lX11 -lprocps -liw
 */

#define _BSD_SOURCE

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iwlib.h>
#include <proc/sysinfo.h>

#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

typedef struct {
    char c[5];
    int i1,i2,i3;
    int i4,i5,i6;
    char out[6];
} CPUS;

#define OUT_TO_CONSOLE 1 // Zero to print in terminal, One to set root windows name
#define WIFI "wlan0"
#define CPUFILE "/proc/stat"
#define FREQFILE "/proc/cpuinfo"
#define NETSPEEDFILE "/proc/net/dev"
#define BATTFILE "/sys/class/power_supply/BAT0/uevent"
#define TEMPFILE1 "/sys/bus/platform/devices/coretemp.0/temp2_input"
#define TEMPFILE2 "/sys/bus/platform/devices/coretemp.0/temp4_input"

static void get_cpu_perc();
static void get_cpu_freq();
static void get_mem_mb();
static void update_speed();
static void get_wifi_strength();
static void get_batt_perc();
static void get_temps();
static void get_day_date();
static void get_uptime();

static Display *dis;

static char cpu_ret[25];
static double freq_ret;
static unsigned long mem_ret;
static char speed_ret[12];
static char wifi_ret[15];
static char batt_ret[20];
static char temps_ret[15];
static char time_ret[25];
static char daydate_ret[7];
static char uptime_ret[15];
static double ti, tj;
static long long recd;

#include "fuzzy-time.c"

// Make sure this value is at least the number of cpus
static CPUS cpus[4];

double time_so_far() {
    struct timeval tp;

    if (gettimeofday(&tp, (struct timezone *) NULL) == -1)
        perror("gettimeofday");
    return ((double) (tp.tv_sec)) +
        (((double) tp.tv_usec) * 0.000001 );
}

void get_cpu_perc() {
    FILE *f1;
	double tf;
	int num = 0, t;

    memset(cpu_ret, '\0', 25);
	tf = time_so_far();
	f1 = fopen(CPUFILE, "r");
    if (f1 != NULL) {
        char line[100];
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, "cpu",3) == 0) {
                sscanf(line, "%s\t%d\t%d\t%d\n", cpus[num].c, &cpus[num].i1, &cpus[num].i2, &cpus[num].i3);
                t = (cpus[num].i1+cpus[num].i2+cpus[num].i3)-(cpus[num].i4+cpus[num].i5+cpus[num].i6);
                sprintf(cpus[num].out, "%.0f%% ", (t / (tf-ti)));
                cpus[num].i4 = cpus[num].i1;
                cpus[num].i5 = cpus[num].i2;
                cpus[num].i6 = cpus[num].i3;
                ++num;
            } else break;
        }
	}
	fclose(f1);

	ti = tf;
    for(t=(num>1)?1:0;t<num;++t)
        strcat(cpu_ret, cpus[t].out);

	return;
}

void get_cpu_freq() {
    FILE *f1;

	f1 = fopen(FREQFILE, "r");
    if (f1 != NULL) {
        char line[100];
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, "cpu MHz", 7) == 0) {
                freq_ret = (float)(atoi(strchr(line, ':') + 2))/1000;
                break;
            }
        }
    } else fputs("::INFO:: \033[0;31mCouldn't Find CPUFILE\033[0m", stderr);
    fclose(f1);
    return;
}

void get_mem_mb() {
    meminfo();
    mem_ret = (kb_main_total-kb_main_free-kb_main_cached-kb_main_buffers)/1024;
    return;
}

void update_speed() {
	FILE *f1;
    char line[256];
	char *vals;

    f1 = fopen(NETSPEEDFILE, "r");
    if (f1 != NULL) {
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, " wlan0", 6) == 0) {
                vals = strchr(line, ':');
                ++vals;
                break;
            }
        }
    } else {
        sprintf(speed_ret, "NET FAIL F");
        return;
    }
    fclose(f1);

	long long last_recv;
	long long down;
	double delta, down_speed, current;

	/* get delta */
	current = time_so_far();
	delta = current - tj;
	if (delta <= 0.0001) {
		sprintf(speed_ret, "NET FAIL T");
		return;
	}
    tj = current;

	last_recv = recd;

	/* bytes packets errs drop fifo frame compressed multicast|bytes ... */
	sscanf(vals, "%lld %*d %*d %*d %*d %*d %*d %*d %*d",
		&down);

	/* if recv is less than last time, an overflow happened */
	if (down < last_recv) last_recv = 0;
	else recd = down;

	/* calculate speeds */
	down_speed = ((recd - last_recv) / delta)*8;
	if(down_speed > 1000000.0)
	    sprintf(speed_ret, "%.2f Mb/s", down_speed/1000000.0);
	else if(down_speed > 1000)
	    sprintf(speed_ret, " %.1f Kb/s", down_speed/1000.0);
    else
        sprintf(speed_ret, " %.0f b/s", down_speed);

    return;
}

void get_wifi_strength() {
    int skfd;
    struct wireless_info *winfo;
    //struct iwreq wrq;

    winfo = (struct wireless_info *) malloc(sizeof(struct wireless_info));
    memset(winfo, 0, sizeof(struct wireless_info));

    skfd = iw_sockets_open();
    if (iw_get_basic_config(skfd, WIFI, &(winfo->b)) > -1) {
        if (iw_get_range_info(skfd, WIFI, &(winfo->range)) >= 0) {
            winfo->has_range = 1;
        }
        if (iw_get_stats(skfd, WIFI, &(winfo->stats),
                &winfo->range, winfo->has_range) >= 0) {
            winfo->has_stats = 1;
        }
        sprintf(wifi_ret, "%d%%", (winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
        /* if (iw_get_ext(skfd, WIFI, SIOCGIWRATE, &wrq) >= 0) {
            sprintf(wifi_ret, "%dMb/s %d%%", wrq.u.bitrate.value/1000000,
             (winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
        } */
    }
    iw_sockets_close(skfd);
    free(winfo);

}

void get_batt_perc() {
    FILE *Batt;
    char  buffer[80];
    char *battstatus, *chargenow, *lastfull;
    unsigned int perc, c1;
    long nowcharge, fullcharge;

    Batt = fopen( BATTFILE, "r" ) ;
    if ( Batt == NULL ) {
        fprintf(stderr, "::INFO:: \033[0;31mCouldn't find %s\033[0m \n", BATTFILE);
        sprintf(batt_ret, "FILE FAIL");
        return;
    } else {
        while(fgets(buffer,sizeof buffer,Batt) != NULL) {
            if(strstr(buffer,"POWER_SUPPLY_STATUS=") != NULL) {
                battstatus = strdup(strstr(buffer, "=")+1);
                battstatus[strlen(battstatus)-1] = '\0';
            } else if(strstr(buffer,"POWER_SUPPLY_CHARGE_FULL=") != NULL) {
                lastfull = strstr(buffer, "=");
                fullcharge = atoi(lastfull+1);
            } else if(strstr(buffer,"POWER_SUPPLY_CHARGE_NOW=") != NULL) {
                chargenow = strstr(buffer, "=");
                nowcharge = atoi(chargenow+1);
            }
        }
        fclose(Batt);
        perc = (nowcharge*100)/fullcharge;
        c1 = (perc > 90) ? 5 : (perc > 30) ? 2 : 6;
        sprintf(batt_ret, "%s &%d%d%%", battstatus, c1, perc);
    }

}

void get_temps() {
    unsigned int  temp1 = 0, temp2 = 0, c1, c2;

    memset(temps_ret, '\0', 15);
    FILE* file1 = fopen(TEMPFILE1, "r");
    if(file1 != NULL)
        fscanf(file1, "%u", &temp1);
    fclose(file1);

    FILE* file2 = fopen(TEMPFILE2, "rb");
    if(file2 != NULL)
        fscanf(file2, "%u", &temp2);
    fclose(file2);

    if(temp1 > 0 && temp2 > 0) {
        c1 = (temp1 > 49000) ? 6 : 5;
        c2 = (temp2 > 49000) ? 6 : 5;
        sprintf(temps_ret, "&%d%d° &%d%d°", c1, temp1/1000, c2, temp2/1000);
    } else sprintf(temps_ret, "?????");
    return;
}

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

void
setstatus(char *str) {
    if(OUT_TO_CONSOLE == 0) {
        printf("\r%s", str);
        fflush(stdout);
    } else {
        XStoreName(dis, DefaultRootWindow(dis), str);
        XSync(dis, False);
    }
}

int
main(void) {
    char status[256];
    unsigned int count = 0;

    if (!(dis = XOpenDisplay(NULL))) {
        fprintf(stderr, "status_output: cannot open display.\n");
        return 1;
    }

    get_day_date();
    get_wifi_strength();
    get_batt_perc();
    get_temps();
    fuzzytime();
    while(1) {
        if(count == 30) {
            get_batt_perc();
            count = 0;
        } else if(count == 2 || count == 17) get_temps();
        else if(count == 4 || count == 19) fuzzytime();
        if((count%2) == 0) get_mem_mb();
        else get_wifi_strength();
        get_cpu_perc();
        get_cpu_freq();
        update_speed();
        get_uptime();

        sprintf(status, "&4¤&3 %s %s &4ð&1 %s &4± %s &4Î&1 %luMiB &4µ&1 %.2f&2 %s &4µ&1 %s &4É&5 %s &4Ï&5 %s &4ê ",
             speed_ret, wifi_ret, batt_ret, temps_ret, mem_ret, freq_ret, cpu_ret, uptime_ret, daydate_ret, time_ret);
        setstatus(status);
        ++count;
        sleep(1);
    }
    XCloseDisplay(dis);

    return 0;
} 
