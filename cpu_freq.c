/* Build with
 * gcc cpu_freq.c -o cpu_freq
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FREQFILE "/proc/cpuinfo"

static void get_cpu_freq();

static char freq_ret[5];

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

int main() {
    while(1) {
        get_cpu_freq();
        printf("\rFREQ = %s", freq_ret);
        fflush(stdout);
        sleep(2);
    }
    return 0;
}
