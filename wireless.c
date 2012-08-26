
/* Build with
 *  gcc wireless.c -o wireless_info -liw
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iwlib.h>

#define WIFI "wlan0"

static void get_wifi_qual();

void get_wifi_qual() {
    // wireless info variables
    int skfd, has_bitrate = 0;
    struct wireless_info *winfo;
    struct iwreq wrq;
    char wi_bitrate[10];

    winfo = (struct wireless_info *) malloc(sizeof(struct wireless_info));
    memset(winfo, 0, sizeof(struct wireless_info));

    skfd = iw_sockets_open();
    if (iw_get_basic_config(skfd, WIFI, &(winfo->b)) > -1) {

        // set present winfo variables
        if (iw_get_stats(skfd, WIFI, &(winfo->stats),
                &winfo->range, winfo->has_range) >= 0) {
            winfo->has_stats = 1;
        }
        if (iw_get_range_info(skfd, WIFI, &(winfo->range)) >= 0) {
            winfo->has_range = 1;
        }
        if (iw_get_ext(skfd, WIFI, SIOCGIWAP, &wrq) >= 0) {
            winfo->has_ap_addr = 1;
            memcpy(&(winfo->ap_addr), &(wrq.u.ap_addr), sizeof(sockaddr));
        }

        // get bitrate
        if (iw_get_ext(skfd, WIFI, SIOCGIWRATE, &wrq) >= 0) {
            memcpy(&(winfo->bitrate), &(wrq.u.bitrate), sizeof(iwparam));
            iw_print_bitrate(wi_bitrate, 16, winfo->bitrate.value);
            has_bitrate = 1;
            printf("\rBitrate = %s ", wi_bitrate);
        }

        // get link quality
        //if (winfo->has_range && winfo->has_stats
        //       && ((winfo->stats.qual.level != 0)
        //       || (winfo->stats.qual.updated & IW_QUAL_DBM))) {
        //    if (!(winfo->stats.qual.updated & IW_QUAL_QUAL_INVALID)) {
                printf("qual = %d ",winfo->stats.qual.qual);
                printf("qual_max = %d ",winfo->range.max_qual.qual);
                printf("qual_percent = %d%%",
                    (winfo->stats.qual.qual*100)/winfo->range.max_qual.qual);
        //    }
        //}

        // get ap mac
        /* if (winfo->has_ap_addr) {
            iw_sawap_ntop(&winfo->ap_addr, ns->ap);
        } */

        // get essid
        /* if (winfo->b.has_essid) {
            if (winfo->b.essid_on) {
                printf("essid = %s\n", winfo->b.essid);
            } else {
                printf("essid = off/any\n");
            }
        } */
        // get channel and freq
        /* if (winfo->b.has_freq) {
            if(winfo->has_range == 1) {
                ns->channel = iw_freq_to_channel(winfo->b.freq, &(winfo->range));
                iw_print_freq_value(ns->freq, 16, winfo->b.freq);
            } else {
                ns->channel = 0;
                ns->freq[0] = 0;
            }
        } */
    }
    iw_sockets_close(skfd);
    free(winfo);
    fflush(stdout);

}

int
main(void) {
    while(1) {
        get_wifi_qual();
        sleep(1);
    }
    return 0;
}
