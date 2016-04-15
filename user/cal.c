#include <inc/lib.h>
#include <inc/stdio.h>
#include <inc/x86.h>
#include <inc/mmu.h>
#include <inc/memlayout.h>

#define CMOS_PORT    0x70
#define CMOS_RETURN  0x71
#define CMOS_STATA   0x0a
#define CMOS_STATB   0x0b
#define CMOS_UIP    (1 << 7)        // RTC update in progress

#define SECS    0x00
#define MINS    0x02
#define HOURS   0x04
#define DAY     0x07
#define MONTH   0x08
#define YEAR    0x09

struct rtcdate {
  int second;
  int minute;
  int hour;
  int day;
  int month;
  int year;
};



void
microdelay(int us)
{
}


static int cmos_read(int reg)
{
  outb(CMOS_PORT,  reg);
  microdelay(200);

  return inb(CMOS_RETURN);
}

static void fill_rtcdate(struct rtcdate *r)
{
  r->second = cmos_read(SECS);
  r->minute = cmos_read(MINS);
  r->hour   = cmos_read(HOURS);
  r->day    = cmos_read(DAY);
  r->month  = cmos_read(MONTH);
  r->year   = cmos_read(YEAR);
}


void cmostime(struct rtcdate *r)
{
  struct rtcdate t1, t2;
  int sb, bcd;

  sb = cmos_read(CMOS_STATB);

  bcd = (sb & (1 << 2)) == 0;

  // make sure CMOS doesn't modify time while we read it
  for (;;) {
    fill_rtcdate(&t1);
    if (cmos_read(CMOS_STATA) & CMOS_UIP)
        continue;
    fill_rtcdate(&t2);
    if (memcmp(&t1, &t2, sizeof(t1)) == 0)
      break;
  }
  if (bcd) {
#define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
    CONV(second);
    CONV(minute);
    CONV(hour  );
    CONV(day   );
    CONV(month );
    CONV(year  );
#undef     CONV
  }

  *r = t1;
  r->year += 2000;
}


// #define JULIAN    1
// #define GREGORIAN 2

/*
 * return the day of the week for particualr date
 * flag JULIAN or GREGORIAN
 * (the divisions are integer divisions that truncate the result)
 * Sun = 0, Mon = 1, etc.
 */
int get_week_day(int day, int month, int year) {
  int a, m, y;

  a = (14 - month) / 12;
  y = year - a;
  m = month + 12*a - 2;

  // if (mode == JULIAN) {
  //   return (5 + day + y + y/4 + (31*m)/12) % 7;
  // }

  return (day + y + y/4 - y/100 + y/400 + (31*m)/12) % 7; // GREGORIAN
}

char *names[] = {"January", "February", "March", "April", "May", "June",
                    "July", "August", "September", "October", "November", "December"};

int months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void printMonth(int month,int year){
  int startingDay = get_week_day(1, month+1, year);
  cprintf("\n  ------------%s-------------\n", names[month]);   // month name
  cprintf("  Sun  Mon  Tue  Wed  Thu  Fri  Sat\n");         // days of week
  int daysInMonth = months[month];
  int dayOfWeek;
  for (dayOfWeek = 0; dayOfWeek < startingDay; ++dayOfWeek)
      cprintf("     ");
  int date;
  for (date = 1; date <= daysInMonth; ++date) {
      cprintf("%5d", date);

      if (++dayOfWeek > 6) {
          cprintf("\n");
          dayOfWeek = 0;
      }
  } // for date

  if (dayOfWeek != 0)
      cprintf("\n");
  return;
}
void umain(int argc , char** argv) {
    int year, month;

    if(argc == 1){
      struct rtcdate r;
      cmostime(&r);
      year = r.year;
      month = r.month-1;
    }
    else if(argc==2){
      year = strtol(argv[1],NULL,10);
      if(year <=0)
      {
        cprintf("Enter a valid year\n");
        return;
      }
    }
    else
    {

      month = strtol(argv[1],NULL,10);
       if(month <=0 || month > 12)
      {
        cprintf("Enter a valid month\n");
        return;
      }
      year = strtol(argv[2],NULL,10);
       if(year <=0)
      {
        cprintf("Enter a valid year\n");
        return;
      }
      month--;
    }

    cprintf("Year: %d\n",year);

    if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
       months[1] = 29;
     if(argc == 1 || argc == 3)
    {

       printMonth(month,year);    
      
    }
    else{
      for (month = 0; month < 12; ++month) {
        printMonth(month,year);
      }

    }
    
  

}