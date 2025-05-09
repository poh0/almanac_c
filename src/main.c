#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#define RESET       "\033[0m"
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */

#define MAX_DATES_IN_MONTH 31
#define MAX_NOTE_LEN 255
#define MAX_LINE_LEN sizeof("XX;") + MAX_NOTE_LEN

// This will be concatenated to home dir except for debug
#define SAVE_DIR_NAME "/.alma/"

// YYYY-MM.txt
#define FILE_NAME_FMT "%04" PRIu16 "-%02" PRIu8 ".txt"

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
    uint8_t mday;
    bool is_sig;
    char *note;
} Date;

typedef struct {
    uint8_t first_weekday;
    uint8_t curr_month;
    uint16_t curr_year;
    uint8_t curr_mday;
    uint8_t cnt_dates;
    Date dates[MAX_DATES_IN_MONTH];
} Calendar;

bool is_leap_year(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

// current month's first day to next month's first day
int days_in_month(int month, int year) {
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    return 30 | ((month & 1) ^ (month >> 3));
}

// Get first day of week of a month using Zeller's congruence
int first_dow_zeller(int month, int year) {

    if (month <= 2) {
        month += 12;
        year -= 1;
    }

    int K = year % 100;
    int J = year / 100;

    int h = ((13 * (month + 1)) / 5 + K + K / 4 + J / 4 + (5 * J)) % 7;

    // Convert Zeller’s result to 0=Sunday, 1=Monday, ...
    int weekday = (h + 6) % 7;

    return weekday;
}

// Split string by a delimeter, results saved to linedata
// nonzero = string was split into n parts successfully
int split_by_delim(char *src, char delim, char **linedata, size_t n) {
    /*
     EXAMPLE
         Split "12.34.56" with delim='.' and n=2:

     Before:
     src ->    [1][2][.][3][4][.][5][6][\0]

     After:
     src ->    [1][2][\0][3][4][.][5][6][\0]
                ^         ^
     linedata  [0]       [1]
     Result:   linedata[0] -> "12", linedata[1] -> "34.56"
    */
    char *delim_ptr;
    linedata[0] = src;
    for (size_t sz = 1; sz < n; sz++) {
        if ((delim_ptr = strchr(linedata[sz-1], delim)) == NULL) {
            return 0;
        }
        linedata[sz-1][delim_ptr - linedata[sz-1]] = '\0';
        linedata[sz] = delim_ptr + 1;
    }
    return 1;
}

void warn_date_doesnt_exist(Calendar *cal) {
    printf("Date doesn't exist.\n");
    printf("%s has only %" PRIu8 " days.\n", MONTHS[cal->curr_month], cal->cnt_dates);
}

// PARAM: char *note
// Pass NULL if we don't want to load note into memory, just mark date as sig
void add_sig_date(Calendar *cal, size_t date, char *note) {
    if (date > cal->cnt_dates || date < 1) {
        return;
    }
    else {
        cal->dates[date - 1].is_sig = true;
        if (note != NULL) {
            cal->dates[date - 1].note = (char*) malloc(strlen(note) + 1);
            strcpy(cal->dates[date - 1].note, note);
        } else {
            cal->dates[date - 1].note = NULL;
        }
    }
}

void remove_sig_date(Calendar *cal, size_t date) {
    if (date > cal->cnt_dates) {
        warn_date_doesnt_exist(cal);
    }
    else {
        cal->dates[date - 1].is_sig = false;
        free(cal->dates[date - 1].note);
        cal->dates[date - 1].note = NULL;
    }
}

void print_sig_date_note(Calendar *cal, size_t date_mday) {
    if (date_mday > cal->cnt_dates) {
        warn_date_doesnt_exist(cal);
    }
    else if (cal->dates[date_mday - 1].is_sig) {
        printf("Note found on %" PRIu8 ". %s:\n", cal->dates[date_mday-1].mday, MONTHS[cal->curr_month]);
        printf("%s\n", cal->dates[date_mday-1].note);
    }
    else {
        printf("Not a significant date.\n\n");
        printf("Add new significant date with:\n");
        printf("\talm sig <date>\n");
    }
}

void print_calendar(Calendar *cal) {
    size_t begin_day_idx = cal->first_weekday;

    // Print month and year
    printf("          %s %" PRIu16 "\n", MONTHS[cal->curr_month], cal->curr_year);

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

    while (current_idx < cal->cnt_dates) {
        /* If we are on sunday, newline */
        if (begin_day_idx == 7) {
            begin_day_idx = 0;
            printf("\n");
            continue;
        }

        else if (current_idx + 1 == cal->curr_mday) {
            if (cal->dates[current_idx].is_sig)
                printf(BOLDGREEN "%3zu!" RESET, ++current_idx);
            else
                printf(BOLDGREEN "%4zu" RESET, ++current_idx);
        }

        else if (cal->dates[current_idx].is_sig) {
            printf(BOLDYELLOW "%4zu" RESET, ++current_idx);
        }

        else {
            printf("%4zu", ++current_idx);
        }

        begin_day_idx++;
    }
    printf("\n");

    if (cal->dates[cal->curr_mday - 1].is_sig && cal->dates[cal->curr_mday - 1].note != NULL) {
        printf("You have a note for today: %s\n", cal->dates[cal->curr_mday-1].note);
    }
} 

void populate_dates(Calendar *cal) {
    Date *date = NULL;
    for (size_t i = 0; i < cal->cnt_dates; i++) {
        date = &cal->dates[i];
        date->mday = i + 1;
        date->is_sig = false;
    }
}

FILE *get_save_file_handle(Calendar *cal, const char *modes) {
#ifndef DEBUG
    char *homedir = getenv("HOME");
    if (homedir == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
#else
    char *homedir = ".";
#endif
    size_t len = strlen(homedir)+ sizeof SAVE_DIR_NAME + sizeof FILE_NAME_FMT - 1;
    char buff[len];
    strcpy(buff, homedir);
    strcat(buff, SAVE_DIR_NAME);
    char filename[sizeof FILE_NAME_FMT];
    snprintf(filename, sizeof filename, FILE_NAME_FMT, cal->curr_year, cal->curr_month + 1);

    if (strcmp(modes, "w") == 0) {
        // just call mkdir, no need to check whether dir already exists.
        mkdir(buff, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    strcat(buff, filename);
    FILE *fp = fopen(buff, modes);
    return fp;
}

// PARAM: size_t mday_note_to_load
// - Pass 0 if modifying a note (removing, adding a note). The function loads every note into memory.
// - Pass a valid day of month to load only a specific day's note
void slurp_sig_dates(Calendar *cal, size_t mday_note_to_load) {

    FILE* file = get_save_file_handle(cal, "r");

    // File not found so no saved notes for the month, we can return
    if (file == NULL) {
        return;
    }

    char line[MAX_LINE_LEN + 1];
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines or lines that are just a newline
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        // Remove trailing newline, if present
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
        }

        char *linedata[2];
        if (!split_by_delim(line, ';', linedata, 2)) {
            continue;
        }

        char *endptr;
        size_t date_mday = (size_t) strtol(linedata[0], &endptr, 10);

        if (*endptr != '\0') continue;

        if (date_mday == mday_note_to_load || !mday_note_to_load) {
            add_sig_date(cal, date_mday, linedata[1]);
        } else {
            add_sig_date(cal, date_mday, NULL);
        }
    }
    fclose(file);
}

void save_new_sig_dates(Calendar *cal) {

    FILE *fp = get_save_file_handle(cal, "w");

    if (fp == NULL) {
        printf("Error opening save file.\n");
        exit(1);
    }

    for (size_t i = 0; i < cal->cnt_dates; i++) {
        if (cal->dates[i].is_sig) {
            fprintf(fp, "%02" PRIu8 ";%s\n",
                    cal->dates[i].mday, cal->dates[i].note);
        }
    }
    fclose(fp);

}

// PARAMS month and year; pass 0 to get current year and month
Calendar init_calendar(int month, int year) {
    time_t t = time(NULL);
    struct tm time_now = *localtime(&t);

    Calendar cal;

    cal.curr_year      = year ? year : (time_now.tm_year + 1900);
    cal.curr_month     = month ? month : time_now.tm_mon;
    cal.curr_mday      = time_now.tm_mday;
    cal.cnt_dates      = days_in_month(cal.curr_month+1, cal.curr_year);
    cal.first_weekday  = first_dow_zeller(cal.curr_month+1, cal.curr_year);
    populate_dates(&cal);

    return cal;
}

void free_notes(Calendar* cal) {
    for(size_t i = 0; i < cal->cnt_dates; i++) {
        if (cal->dates[i].is_sig && cal->dates[i].note != NULL) {
            free(cal->dates[i].note);
        }
    }
}

int main(int argc, char **argv) {

    // TODO:
    // alm -<n>, +<n> to show multiple months at once.
    // -m, -y, --help, etc.
    // Consider adding GNU readline or similar very soon.

    // Show the note of a date
    if (argc == 2) {
        char *endptr;
        int date = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            printf("Invalid input, please enter an integer for date.\n");
            return 1;
        }
        Calendar cal = init_calendar(0, 0);
        slurp_sig_dates(&cal, date);
        print_sig_date_note(&cal, date);
        free_notes(&cal);
    }

    // Add a note to a date
    else if (argc > 2 && strncmp("sig", argv[1], 3) == 0) {
        char *endptr;
        int date = strtol(argv[2], &endptr, 10);

        if (*endptr != '\0') {
            printf("Invalid input, please enter an integer for date.\n");
            return 1;
        }

        Calendar cal = init_calendar(0, 0);

        if (date < 1 || date > cal.cnt_dates) {
            warn_date_doesnt_exist(&cal);
            return 0;
        }

        char note[MAX_NOTE_LEN + 1];

        printf("Enter your note (max 255 characters): ");

        if (fgets(note, sizeof(note), stdin) != NULL) {
            size_t len = strlen(note);

            if (len > 0 && note[len - 1] != '\n') {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Warning: input truncated to 255 characters.\n");
            }
            else {
                note[len - 1] = '\0';
            }

            slurp_sig_dates(&cal, 0);
            add_sig_date(&cal, date, note);
            save_new_sig_dates(&cal);
            free_notes(&cal);
        }
        else {
            printf("Error reading note.\n");
            return 1;
        }
    }

    // Remove note of a date 
    else if (argc > 2 && strncmp("rm", argv[1], 2) == 0) {
        char *endptr;
        int date = strtol(argv[2], &endptr, 10);

        if (*endptr != '\0') {
            printf("Invalid input, please enter an integer for date.\n");
            return 1;
        }

        Calendar cal = init_calendar(0, 0);

        if (date < 1 || date > cal.cnt_dates) {
            warn_date_doesnt_exist(&cal);
            return 0;
        }

        slurp_sig_dates(&cal, 0);

        remove_sig_date(&cal, date);
        save_new_sig_dates(&cal);

        printf("Note removed.\n");
        free_notes(&cal);
    }

    // No args were passed, just print calendar
    else {
        Calendar cal = init_calendar(0, 0);
        slurp_sig_dates(&cal, cal.curr_mday);
        print_calendar(&cal);
        free_notes(&cal);
    }

    return 0;
} 
