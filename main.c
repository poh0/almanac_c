#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define str(x) #x
#define xstr(x) str(x)

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
    Date *dates;
    Sig_date *sig_dates;
    Date *currDate;
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
    printf("          Jan 2022\n");
    for (size_t i = MON; i <= SUN; i++) {
        printf("%4s", weekday_to_cstr(i));
    }
    printf("\n");
    size_t j = 0;
    while (j < calendar->cnt_dates) {
        for (size_t l = 0; l < 7; l++) {
            j++;
            printf("%4zu", j);
        }
        printf("\n");
    }
    printf("\n");
} 

void init_calendar(Calendar *cal) {

    /* TODO: actual current month & date */

    Date currDate = {
        .is_sig = false,
        .index = 22
    };

    cal->currMonth = JUL;
    cal->cnt_dates = 30;
    cal->sig_dates = NULL;
    cal->currDate = &currDate;
}          

int main(int argc, char** argv) {
    
    // Handle ./almanac <date_num>
    
    // Handle ./almanac sig <date_num>
    if (argc > 2 && strcmp("sig", argv[1]) != 0) {
        printf("Usage: almanac sig <date_num>\n");
        exit(0);
    }

    Calendar *cal = malloc(sizeof(Calendar));
    init_calendar(cal);
    print_calendar(cal);
    free(cal);
    return 0;
} 
