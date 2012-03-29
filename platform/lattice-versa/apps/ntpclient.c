/*
 * ntpclient.c  -  ntpclient for uip-1.0
 *
 * This file includes a simple implementation of the ntp protocol
 * as well as some helper functions to keep track of of the accurate 
 * time and some time format conversion functions.
 *
 * This code does not follow the uip philisophy of using at most
 * 16 bit arithemtics. For simplicity reasons it also uses 32 bit
 * arithmetic and thus requires a compiler supporting this. Gcc does.
 *
 * (c) 2006 by Till Harbaum <till@harbaum.org>
 * time conversion part based on code published by 
 * peter dannegger - danni(at)specs.de on mikrocontroller.net
 */


#include "contiki.h"
#include "contiki-net.h"
#include "ntpclient.h"
#include "pdhcp.h"

#include <stdint.h>
#include <stdio.h>

// enable this to get plenty of debug output via stdout
#define NTP_DEBUG
#define MAX_RETRIES 6

#define FIRSTYEAR     1900    // start year
#define FIRSTDAY      1	      // 1.1.1900 was a Monday (0 = Sunday)

enum {
  EVENT_NEW_SERVER=0
};

PROCESS(ntp_process, "NTP process");
static struct etimer timer;

static const char *month_names[] = {
"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static const char *weekday_names[] = {
"Sun", "Mon", "Tues", "Wed", 
"Thur", "Fri", "Sat" };

static const uint8_t DayOfMonth[] = { 31, 29, 31, 30, 31, 30, 
			       31, 31, 30, 31, 30, 31 };

struct ntpstate {
#define STATE_UNUSED 0
#define STATE_NEW    1
#define STATE_ASKING 2
#define STATE_DONE   3
#define STATE_ERROR  4
  u8_t state;
  u8_t tmr;
  u8_t retries;
  uip_ipaddr_t ipaddr;
  struct uip_udp_conn *resolv_conn;
};

#ifndef UIP_CONF_NTP_ENTRIES
#define NTP_ENTRIES 4
#else /* UIP_CONF_RESOLV_ENTRIES */
#define NTP_ENTRIES UIP_CONF_NTP_ENTRIES
#endif /* UIP_CONF_RESOLV_ENTRIES */

static struct ntpstate ntpservers[NTP_ENTRIES];
static struct etimer timer;

#if UIP_BYTE_ORDER == UIP_BIG_ENDIAN
static inline uint32_t ntp_read_u32(uint32_t val) { return val; }
#else
static inline uint32_t ntp_read_u32(uint32_t val) { 
  uint8_t tmp, *d = (uint8_t*)&val;
  tmp = d[0]; d[0] = d[3]; d[3] = tmp;
  tmp = d[1]; d[1] = d[2]; d[2] = tmp;
  return val; 
}
#endif

#define NTP_PORT    123

#define FLAGS_LI_NOWARN    (0<<6)   
#define FLAGS_LI_61SEC     (1<<6)   
#define FLAGS_LI_59SEC     (2<<6)
#define FLAGS_LI_ALARM     (3<<6)

#define FLAGS_VN(n)        ((n)<<3)

// we only support client mode
#define FLAGS_MODE_CLIENT  (3)

static void newdata(void);
static void check_entries(void);

// the ntp time message
struct ntp_msg {
  uint8_t flags;
  uint8_t stratum;
  uint8_t poll;
  int8_t precision;

  uint16_t root_delay[2];
  uint16_t root_dispersion[2];
  uint32_t reference_identifier;

  // timestamps:
  uint32_t reference_timestamp[2];
  uint32_t originate_timestamp[2];
  uint32_t receive_timestamp[2];
  uint32_t transmit_timestamp[2];
};

void ntpclient_conf(const uip_ipaddr_t *ntpserver)
{
  static uip_ipaddr_t server;
  uip_ipaddr_copy(&server, ntpserver);
  process_post(&ntp_process, EVENT_NEW_SERVER, &server);
}

process_event_t ntpclient_event_updated;

PROCESS_THREAD(ntp_process, ev, data)
{
  // uip_ipaddr_t * addr;
  struct ntpstate *stateptr;
  static u8_t i;
  
  PROCESS_BEGIN();
  ntpclient_event_updated = process_alloc_event();
  
    for(i = 0; i < NTP_ENTRIES; ++i) {
		ntpservers[i].state = STATE_UNUSED;
	}	
  
  
  while(1) {
  
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_MSG) {
		//resolv_query("pool.ntp.org");
		// printf("event: PROCESS_EVENT_MSG\n");		
   		
	} if (ev == EVENT_NEW_SERVER) {
		stateptr = &ntpservers[0];	
		uip_ipaddr_copy(&(stateptr->ipaddr), (uip_ipaddr_t *)data);
	
		stateptr->state = STATE_NEW;
		
		if(stateptr->resolv_conn != NULL) {
			uip_udp_remove(stateptr->resolv_conn);
		}
		stateptr->resolv_conn = udp_new((uip_ipaddr_t *)data, UIP_HTONS(NTP_PORT), NULL);
	
		// request a callback when we have the buffer available
	    tcpip_poll_udp(stateptr->resolv_conn);
		
		printf("NTP: configured for new server \n");
				
    } else if(ev == tcpip_event) {
	   // printf("event: TCPIP_EVENT\n");
        if(uip_udp_conn->rport == UIP_HTONS(NTP_PORT)) {
			if(uip_poll()) {
			  check_entries();
			}
			if(uip_newdata()) {
			  newdata();
			}
      }
    } else if(ev == PROCESS_EVENT_TIMER) {
	    // printf("ntp PROCESS_EVENT_TIMER\n");
		
		stateptr = &ntpservers[0];
		if (stateptr->resolv_conn != NULL) {
			tcpip_poll_udp(stateptr->resolv_conn);
		}		
		
	}
    
  }

  PROCESS_END();
}


void
ntp_tmtime(uint32_t sec, struct ntp_tm *tm) {
  uint16_t day;
  uint8_t year;
  uint16_t dayofyear;
  uint8_t leap400;
  uint8_t month;

  // adjust timezone
  // sec += 3600 * NTP_TZ;

  tm->second = sec % 60;
  sec /= 60;
  tm->minute = sec % 60;
  sec /= 60;
  tm->hour = sec % 24;
  day = sec / 24;

  tm->weekday = (day + FIRSTDAY) % 7;		// weekday

  year = FIRSTYEAR % 100;			// 0..99
  leap400 = 4 - ((FIRSTYEAR - 1) / 100 & 3);	// 4, 3, 2, 1

  for(;;){
    dayofyear = 365;
    if( (year & 3) == 0 ){
      dayofyear = 366;					// leap year
      if( year == 0 || year == 100 || year == 200 )	// 100 year exception
        if( --leap400 )					// 400 year exception
          dayofyear = 365;
    }
    if( day < dayofyear )
      break;
    day -= dayofyear;
    year++;					// 00..136 / 99..235
  }
  tm->year = year + FIRSTYEAR / 100 * 100;	// + century

  if( dayofyear & 1 && day > 58 )		// no leap year and after 28.2.
    day++;					// skip 29.2.

  for( month = 1; day >= DayOfMonth[month-1]; month++ )
    day -= DayOfMonth[month-1];

  tm->month = month;				// 1..12
  tm->day = day + 1;				// 1..31
}
/*---------------------------------------------------------------------------*/
static void
send_request(void)
{
  // struct ntp_time time;
  struct ntp_msg *m = (struct ntp_msg *)uip_appdata;

  // build ntp request
  m->flags = FLAGS_LI_ALARM | FLAGS_VN(4) | FLAGS_MODE_CLIENT;
  m->stratum = 0;                    // unavailable
  m->poll = 4;                       // 2^4 = 16 sec
  m->precision = -6;                 // 2^-6 = 0.015625 sec = ~1/100 sec

  m->root_delay[0] = UIP_HTONS(1);       // 1.0 sec
  m->root_delay[1] = 0;

  m->root_dispersion[0] = UIP_HTONS(1);  // 1.0 sec
  m->root_dispersion[1] = 0;

  // we don't have a reference clock
  m->reference_identifier = 0;
  m->reference_timestamp[0] = m->reference_timestamp[1] = 0;

  // we don't know any time
  m->originate_timestamp[0] = m->originate_timestamp[1] = 0;
  m->receive_timestamp[0]   = m->receive_timestamp[1]   = 0;

  // put our own time into the transmit frame (whatever our own time is)
  // clock_get_ntptime(&time);
  // m->transmit_timestamp[0]  = ntp_read_u32(time.seconds);
  // m->transmit_timestamp[1]  = ntp_read_u32(time.fraction);
  m->transmit_timestamp[0]  = 0;
  m->transmit_timestamp[1]  = 0;

  // and finally send request
  uip_send(uip_appdata, sizeof(struct ntp_msg));
}
/*---------------------------------------------------------------------------*/
static uint8_t
parse_msg(struct ntp_time *time)
{
  struct ntp_msg *m = (struct ntp_msg *)uip_appdata;

  // check correct time len
  if(uip_datalen() != sizeof(struct ntp_msg))
    return 0;

  // adjust endianess
  time->seconds = ntp_read_u32(m->transmit_timestamp[0]);
  time->fraction = ntp_read_u32(m->transmit_timestamp[1]) >> 16;

  return 1;
}

static void
check_entries(void)
{
   struct ntpstate *stateptr;
	// printf("ntpclient: check_entries\n");
	
	// TODO: find ntpservers entry from uip state
	stateptr = &ntpservers[0];

    if(stateptr->state == STATE_NEW || stateptr->state == STATE_ASKING) {
		if(stateptr->state == STATE_ASKING) {
			if(--stateptr->tmr == 0) {
				if(++stateptr->retries == MAX_RETRIES) {
					stateptr->state = STATE_ERROR;
					printf("NTP: giving up on server %d.%d.%d.%d\n", 
						stateptr->ipaddr.u8[0],
						stateptr->ipaddr.u8[1],
						stateptr->ipaddr.u8[2],
						stateptr->ipaddr.u8[3]);
						
						
						// ask pdhcp to resolve another sever
						process_post(&dhcp_process, PROCESS_EVENT_MSG, NULL);

						
				}
				stateptr->tmr = stateptr->retries;
			} else {
				  /*	  printf("Timer %d\n", stateptr->tmr);*/
				  /* Its timer has not run out, so we move on to next
					 entry. */
				// continue;
				return;
			}
		} else {
			stateptr->state = STATE_ASKING;
			stateptr->tmr = 1;
			stateptr->retries = 0;
		}
		printf("NTP: sending request to %d.%d.%d.%d\n", 
			stateptr->ipaddr.u8[0],
	        stateptr->ipaddr.u8[1],
	        stateptr->ipaddr.u8[2],
	        stateptr->ipaddr.u8[3]);

	    etimer_set(&timer, CLOCK_SECOND);
		send_request();
	}
	
	//printf("ntpclient: check_entries (completed)\n");
}

static void newdata(void)
{
  
  struct ntp_time stime;
  struct ntp_tm tm;
  struct ntpstate *stateptr;

  //printf("ntpclient: newdata\n");
  
    if(uip_newdata() && parse_msg(&stime)) {
		// stime contains seconds and fraction
		stateptr = &ntpservers[0];
	
		/* calculate time between send/receive */
		/* don't use a signal that traveled longer than one second */
		stateptr->state = STATE_DONE;
	
		// convert ntp seconds since 1900 into useful time structure
		ntp_tmtime(stime.seconds, &tm);

		printf("NTP: utctime: %s %d. %s %d %d:%02d:%02d \n", 
		 weekday_names[tm.weekday],
		 tm.day, month_names[tm.month-1], tm.year,
		 tm.hour, tm.minute, tm.second );
		
		printf("NTP: setting hw time\n");
		clock_set_ntptime(&tm);
		
		// send time-updated message
		process_post(PROCESS_BROADCAST, ntpclient_event_updated, NULL);

	}
	
	//printf("ntpclient: newdata (completed)\n");
	
}


