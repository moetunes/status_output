#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <proc/sysinfo.h>

static void get_mem_mb();

static char mem_ret[10];

void get_mem_mb() {
    meminfo();
    sprintf(mem_ret, "%d", (kb_main_total-kb_main_free-kb_main_cached-kb_main_buffers)/1024);
}

int main() {
    while(1) {
        get_mem_mb();
        printf("MEM %s\n", mem_ret);
        sleep(2);
    }
    return 0;
}
