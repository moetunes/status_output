/* Build with
 * gcc cpu_usage.c -o cpu_usage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    char c[10];
    int i1,i2,i3;
    int i4,i5,i6;
    char out[5];
} CPUS;

static void get_cpu_perc();

static char cpu_ret[20];
static double ti;

static CPUS cpus[4];

double time_so_far()
{
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

    memset(cpu_ret, '\0', 20);
	tf = time_so_far();
	f1 = fopen("/proc/stat", "r");
    if (f1 != NULL) {
        char line[100];
        while(fgets(line, sizeof line, f1) != NULL) {
            if (strncmp(line, "cpu",3) == 0) {
                sscanf(line, "%s\t%d\t%d\t%d\n", cpus[num].c, &cpus[num].i1, &cpus[num].i2, &cpus[num].i3);
                t = (cpus[num].i1+cpus[num].i2+cpus[num].i3)-(cpus[num].i4+cpus[num].i5+cpus[num].i6);
                sprintf(cpus[num].out, "%.1f%% ", (t / (tf-ti)));
                cpus[num].i4 = cpus[num].i1;
                cpus[num].i5 = cpus[num].i2;
                cpus[num].i6 = cpus[num].i3;
                ++num;
            } else break;
        }
	}
	fclose(f1);

	ti = tf;
	t = (num < 1) ? 0 : 1;
    for(t;t<num;++t)
        strcat(cpu_ret, cpus[t].out);

	return;
}

int main() {
    while(1) {
        get_cpu_perc();
        printf("\rCPU %s", cpu_ret);
        fflush(stdout);
        sleep(2);
    }
    return 0;
}
