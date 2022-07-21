#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define RESET   "\033[0m"
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW   "\033[1m\033[33m"      /* Bold Yellow */

const char* const MONTHS[] = { 
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec" 
};

const char* const WEEKDAYS[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

typedef struct {
    bool is_sig;
    bool is_current;
    size_t mday;
} Date;

typedef struct {
    Date *date;
    const char *note; // eg. "Mike's birthday 13:00"
} Sig_Date;

typedef struct {
    size_t first_weekday;
    size_t curr_month;
    size_t curr_year;
    size_t curr_date;
    size_t cnt_dates; 
    Sig_Date sig_dates[32];
    Date dates[32];
} Calendar; 

void print_sig_date(Sig_Date *sig_date) {
    (void) sig_date;
} 

void print_calendar(Calendar *calendar) {

    size_t begin_day_idx = calendar->first_weekday;

    // Print month and year
    printf("          %s %zu\n", MONTHS[calendar->curr_month], calendar->curr_year);

    // Print weekdays
    for (size_t i = 0; i < 7; i++) {
        printf("%4s", WEEKDAYS[i]);
    }
 
    printf("\n");

    /* Shift cursor to the month's first weekday */
    for (size_t i = 0; i < begin_day_idx; i++) {
        printf("%4s", " ");
    }

    size_t current_idx = 0;

    while (current_idx < calendar->cnt_dates) {
        /* If we are on sunday, newline */
        if (begin_day_idx == 7) {
            begin_day_idx = 0;
            printf("\n");
            continue;
        }

        else if (current_idx + 1 == calendar->curr_date) {
            if (calendar->dates[current_idx].is_sig)
                printf(BOLDGREEN "%3zu!" RESET, ++current_idx);
            else
                printf(BOLDGREEN "%4zu" RESET, ++current_idx);
        }

        else if (calendar->dates[current_idx].is_sig) {
            printf(BOLDYELLOW "%4zu" RESET, ++current_idx);
        }

        else {
            printf("%4zu", ++current_idx);
        }

        begin_day_idx++;
    }
    printf("\n");
} 

void populate_dates(Calendar *cal) {
    Date *date = NULL;
    for (size_t i = 0; i < cal->cnt_dates; i++) {
        date = &cal->dates[i];
        date->mday = i + 1;
        date->is_current = (date->mday == cal->curr_date);
        date->is_sig = false;
    }
    // Mock data
    cal->dates[2].is_sig = true;
}

// we take time difference in seconds from
// current month's first day to next month's first day
size_t days_in_month(struct tm time_now) {

    struct tm next_month = { 0 };
    next_month.tm_year = time_now.tm_year;
    next_month.tm_mon = time_now.tm_mon + 1;
    next_month.tm_mday = 1;
    next_month.tm_isdst = -1;

    struct tm this_month = { 0 };
    this_month.tm_year = time_now.tm_year;
    this_month.tm_mon = time_now.tm_mon;
    this_month.tm_mday = 1;
    this_month.tm_isdst = -1;

    time_t next_month_t = mktime(&next_month);
    time_t this_month_t = mktime(&this_month);

    double dDays = difftime(next_month_t, this_month_t) / 86400;
    size_t days = (size_t) dDays;
    return days;
}

size_t get_weekday(struct tm time_now) {

    struct tm this_month = { 0 };
    this_month.tm_year = time_now.tm_year;
    this_month.tm_mon = time_now.tm_mon;
    this_month.tm_mday = 1;
    this_month.tm_isdst = -1;

    mktime(&this_month);
    return this_month.tm_wday;
}

void init_calendar(Calendar *cal) {

    time_t t = time(NULL);
    struct tm time_now = *localtime(&t);

//  struct tm time_now = { 0 };
//  time_now.tm_year = 2005 - 1900;
//  time_now.tm_mon = 6;
//  time_now.tm_mday = 24;
//  time_now.tm_isdst = -1;

    cal->curr_year      = time_now.tm_year + 1900;
    cal->curr_month     = time_now.tm_mon;
    cal->curr_date      = time_now.tm_mday;
    cal->cnt_dates      = days_in_month(time_now);
    cal->first_weekday  = get_weekday(time_now);

    populate_dates(cal);
}          

int main(int argc, char** argv) {
    
    // Handle ./almanac <date_num>
    
    // Handle ./almanac sig <date_num>
    if (argc > 2 && strcmp("sig", argv[1]) != 0) {
        printf("Usage: almanac sig <date_num>\n");
        exit(0);
    }

    Calendar *cal = (Calendar*) malloc(sizeof(Calendar));
    init_calendar(cal);
    print_calendar(cal);
    free(cal);
    return 0;
} 
