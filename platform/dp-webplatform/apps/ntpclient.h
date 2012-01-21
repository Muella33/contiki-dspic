/*
 * ntpclient.h
 */
#ifndef __NTPCLIENT_H__
#define __NTPCLIENT_H__

#include "contiki.h"
#include "contiki-net.h"

PROCESS_NAME(ntp_process);

//event broadcast when RTC is updated
CCIF extern process_event_t ntpclient_event_updated; 

struct ntp_time {
  uint32_t seconds;   // full seconds since 1900
  uint16_t fraction;  // fractions of a second in x/65536
};

struct ntp_tm {
  // date
  uint16_t year;   // year
  uint8_t month;   // month within year (1..12)
  uint8_t day;     // day within month (1..31)
  uint8_t weekday; // day within week (0..6)
  // time
  uint8_t hour;    // hour within day (0..23)
  uint8_t minute;  // minute within hour (0..59)
  uint8_t second;  // second within minute (0..59)
};

extern void ntpclient_conf(const uip_ipaddr_t *ntpserver);
extern void ntpclient_init(void);
extern void ntpclient_appcall(void);
extern void ntp_tmtime(uint32_t sec, struct ntp_tm *tm);

// to be provided by clock-arch:
extern void clock_set_ntptime(struct ntp_tm *time);

#endif /* __NTPCLIENT_H__ */
