#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */
#define BOLDWHITE   "\033[1m\033[33m"      /* Bold Yellow */

typedef enum {
    JAN = 0,
    FEB,
    MAR,
    APR,
    MAY,
    JUN,
    JUL,
} Month; 

typedef enum {
    MON = 0,
    TUE,
    WED,
    THU,
    FRI,
    SAT,
    SUN
} Weekday; 

typedef struct {
    bool is_sig;
    bool is_current;
    size_t index;
} Date;

typedef struct {
    Date *date;
    const char *note; // eg. "Mike's birthday 13:00"
} Sig_date;

typedef struct {
    Month currMonth;
    Weekday firstWeekday;
    size_t cnt_dates; 
    size_t curr_date;
    Sig_date *sig_dates;
    Date dates[32];
} Calendar; 

const char *weekday_to_cstr(Weekday weekday) {
    switch (weekday) {
        case MON:
            return "MON";
        case TUE:
            return "TUE";
        case WED:
            return "WED";
        case THU:
            return "THU";
        case FRI:
            return "FRI";
        case SAT:
            return "SAT";
        case SUN:
            return "SUN";
        default:
            printf("Unreachable\n");
            return NULL;
    };
}

void print_sig_date(Sig_date *sig_date) {
    (void) sig_date;
} 

void print_calendar(Calendar *calendar) {

    size_t begin_day_idx = calendar->firstWeekday;

    printf("          Jan 2022\n");
    for (size_t i = MON; i <= SUN; i++) {
        printf("%4s", weekday_to_cstr(i));
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
        else if (current_idx == calendar->curr_date) {
            printf(GREEN "%4zu" RESET, current_idx++);
        }
        else if (calendar->dates[current_idx].is_sig) {
            printf(BOLDWHITE "%4zu" RESET, current_idx++);
        }
        else {
            printf("%4zu", current_idx++);
        }
        begin_day_idx++;
    }
    printf("\n");
} 

void populate_dates(Calendar *cal) {
    Date *date = NULL;
    for (size_t i = 0; i < cal->cnt_dates; i++) {
        date = &cal->dates[i];
        date->index = i;
        date->is_current = (date->index == cal->curr_date);
        date->is_sig = false;
    }
    // Here for reference
    cal->dates[13].is_sig = true;
}

void init_calendar(Calendar *cal) {

    /* TODO: actual current month & date */

    cal->cnt_dates = 30;
    cal->curr_date = 22;
    populate_dates(cal);
    cal->firstWeekday = WED;
    cal->currMonth = JUL;
    cal->sig_dates = NULL;
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
