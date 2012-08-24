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
#define BATTFILE "/sys/class/power_supply/BAT0/uevent"
#define TEMPFILE1 "/sys/bus/platform/devices/coretemp.0/temp2_input"
#define TEMPFILE2 "/sys/bus/platform/devices/coretemp.0/temp4_input"

static void get_cpu_perc();
static void get_cpu_freq();
static void get_mem_mb();
static void get_wifi_strength();
static void get_batt_perc();
static void get_temps();
static void mktimes();

static Display *dis;

static char cpu_ret[25];
static char freq_ret[5];
static char mem_ret[10];
static char wifi_ret[5];
static char batt_ret[20];
static char temps_ret[10];
static char time_ret[25];

#include "fuzzy-time.c"

static double ti;

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
	f1 = fopen(CPUFILE, "rb");
    if (f1 != NULL) {
        char line[100];
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, "cpu",3) == 0) {
                sscanf(line, "%s\t%d\t%d\t%d\n", cpus[num].c, &cpus[num].i1, &cpus[num].i2, &cpus[num].i3);
                t = (cpus[num].i1+cpus[num].i2+cpus[num].i3)-(cpus[num].i4+cpus[num].i5+cpus[num].i6);
                sprintf(cpus[num].out, "%.0f%% ", (t / ((tf-ti) * 100)) * 100);
                cpus[num].i4 = cpus[num].i1;
                cpus[num].i5 = cpus[num].i2;
                cpus[num].i6 = cpus[num].i3;
                ++num;
            } else break;
        }
	}
	fclose(f1);

	ti = tf;
	t = (num > 1) ? 1 : 0;
    for(t;t<num;++t)
        strcat(cpu_ret, cpus[t].out);

	return;
}

void get_cpu_freq() {
    FILE *f1;
	double freq;

    memset(freq_ret, '\0', 5);
	f1 = fopen(FREQFILE, "rb");
    if (f1 != NULL) {
        char line[100];
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, "cpu MHz", 7) == 0) {
                freq = atoi(strchr(line, ':') + 2);
                break;
            }
        }
    } else puts("Wrong file");
    fclose(f1);
    sprintf(freq_ret, "%.2f", (freq/1000));
    return;
}

void get_mem_mb() {
    meminfo();
    sprintf(mem_ret, "%dMiB",
     (kb_main_total-kb_main_free-kb_main_cached-kb_main_buffers)/1024);
    return;
}

void get_wifi_strength() {
    int skfd;
    struct wireless_info *winfo;

    winfo = (struct wireless_info *) malloc(sizeof(struct wireless_info));
    memset(winfo, 0, sizeof(struct wireless_info));

    skfd = iw_sockets_open();
    if (iw_get_basic_config(skfd, WIFI, &(winfo->b)) > -1) {
        if (iw_get_stats(skfd, WIFI, &(winfo->stats),
                &winfo->range, winfo->has_range) >= 0) {
            winfo->has_stats = 1;
        }
        if (iw_get_range_info(skfd, WIFI, &(winfo->range)) >= 0) {
            winfo->has_range = 1;
        }
        sprintf(wifi_ret, "%d%%",
            (winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
    }
    iw_sockets_close(skfd);
    free(winfo);

}

void get_batt_perc() {
    FILE *Batt;
    char  buffer[80];
    char *battstatus, *chargenow, *lastfull;
    unsigned int perc;
    long nowcharge, fullcharge;

    Batt = fopen( BATTFILE, "rb" ) ;
    if ( Batt == NULL ) {
        fprintf(stderr, "\t\033[0;31mCouldn't find %s\033[0m \n", BATTFILE);
        return;
    } else {
        while(fgets(buffer,sizeof buffer,Batt) != NULL) {
            if(strstr(buffer,"POWER_SUPPLY_STATUS=") != NULL) {
                battstatus = strndup(strstr(buffer, "=")+1, strlen(strstr(buffer, "=")+1)-1);
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
        sprintf(batt_ret, "%s &4%d%%", battstatus, perc);
    }

}

void get_temps() {
    int  temp1 = 0, temp2 = 0;

    memset(temps_ret, '\0', 10);
    FILE* file1 = fopen(TEMPFILE1, "rb");
    if(file1 != NULL)
        fscanf(file1, "%d", &temp1);
    fclose(file1);

    FILE* file2 = fopen(TEMPFILE2, "rb");
    if(file2 != NULL)
        fscanf(file2, "%d", &temp2);
    fclose(file2);

    if(temp1 > 0 && temp2 > 0)
        sprintf(temps_ret, "%d° %d°", temp1/1000, temp2/1000);
    else sprintf(temps_ret, "?????");
    return;
}

void mktimes() {
    struct tm tm; 
    time_t    time_value = time(0);
    tm = *localtime(&time_value);
    char secs[3]; char minutes[3]; char hours[3]; char arvo[3];
    
    if(tm.tm_hour >12) {
        sprintf(hours, "%d", tm.tm_hour-12);
        strncpy(arvo, "pm", 3);
    } else {
        sprintf(hours, "%d", tm.tm_hour);
        strncpy(arvo, "am", 3);
    }
    if(tm.tm_min > 9) sprintf(minutes, "%d", tm.tm_min);
    else sprintf(minutes, "0%d", tm.tm_min);
    if(tm.tm_sec > 9) sprintf(secs, "%d", tm.tm_sec);
    else sprintf(secs, "0%d", tm.tm_sec);
    
    sprintf(time_ret, "%s:%s:%s %s", hours, minutes, secs, arvo);
    return;
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

    while(1) {
        if(count == 0 || count == 30) {
            get_batt_perc();
            count = 0;
        }
        if(count == 0 || count == 15) {
            get_temps();
            fuzzytime();
        }
        if((count%2) == 0) get_mem_mb();
        else get_wifi_strength();
        get_cpu_perc();
        get_cpu_freq();
        //mktimes();

        sprintf(status, "&C&8ð&7 %s&R &8¤&5 %s &B2&8±&7 %s &8Î&7 %s &8µ&5 %s&7 %s &B0&8É&5 %s &8ê ",
             batt_ret, wifi_ret, temps_ret, mem_ret, freq_ret, cpu_ret, time_ret);
        setstatus(status);
        ++count;
        sleep(1);
    }
    XCloseDisplay(dis);

    return 0;
} 
