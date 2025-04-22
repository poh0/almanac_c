#include <stdio.h>
#include <stdint.h>
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
    size_t mday;
    char *note;
} Date;

typedef struct {
    size_t first_weekday;
    size_t curr_month;
    size_t curr_year;
    size_t curr_mday;
    size_t cnt_dates;
    Date dates[MAX_DATES_IN_MONTH];
} Calendar; 

bool is_leap_year(size_t year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

// current month's first day to next month's first day
size_t days_in_month(size_t month, size_t year) {
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    return 30 | ((month & 1) ^ (month >> 3));
}

// Get first day of week of a month
size_t first_dow_zeller(size_t month, size_t year) {

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
// nonzero = string was split into <size> parts successfully
int split_by_delim(char *src, char delim, char **linedata, size_t size) {

    // 22.22.22\0
    // 22\022.22\0
    // 22\022\022\0

    char *tok = strchr(src, delim);
    if (tok == NULL) {
        return 0;
    }
    linedata[0] = src;
    linedata[0][tok - src] = '\0';
    linedata[1] = tok + 1;

    size_t sz = 2;
    while (sz < size) {
        if ((tok = strchr(linedata[sz-1], delim)) == NULL) {
            return 0;
        }
        linedata[sz-1][tok - linedata[sz-1]] = '\0';
        linedata[sz] = tok + 1;
    }
    return 1;
}

void warn_date_doesnt_exist(Calendar *cal) {
    printf("Date doesn't exist.\n");
    printf("%s has only %zu days.\n", MONTHS[cal->curr_month], cal->cnt_dates);
}

// PARAM: char *note
// Pass NULL if we don't want to load note into memory
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

void print_sig_date_note(size_t date_mday, Calendar *cal) {
    if (date_mday > cal->cnt_dates) {
        warn_date_doesnt_exist(cal);
    }
    else if (cal->dates[date_mday - 1].is_sig) {
        printf("Note found on %zu. %s:\n", cal->dates[date_mday-1].mday, MONTHS[cal->curr_month]);
        printf("%s\n", cal->dates[date_mday-1].note);
    }
    else {
        printf("Not a significant date.\n\n");
        printf("Add new significant date with:\n");
        printf("\talm sig <date>\n");
    }
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

        else if (current_idx + 1 == calendar->curr_mday) {
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

    if (calendar->dates[calendar->curr_mday - 1].is_sig) {
        printf("You have a note for today: %s\n", calendar->dates[calendar->curr_mday-1].note);
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


// PARAM: size_t mday_note_to_load
// - Pass 0 if modifying a note (removing, adding a note). The function loads every note into memory.
// - Pass a valid day of month to load only a specific day's note
void slurp_sig_dates(Calendar *cal, size_t mday_note_to_load) {
#ifndef DEBUG
    char *homedir = getenv("HOME");
    if (homedir == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
#else
    char *homedir = ".";
#endif
    size_t len = strlen(homedir)+ sizeof SAVE_DIR_NAME + sizeof "%04zu-%02zu.txt" - 1;
    char buff[len];
    strcpy(buff, homedir);
    strcat(buff, SAVE_DIR_NAME);
    char filename[sizeof "%04zu-%02zu.txt"];
    snprintf(filename, sizeof filename, "%04zu-%02zu.txt", cal->curr_year, cal->curr_month);
    strcat(buff, filename);

    FILE* file = fopen(buff, "r");

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
        if (len > 0 && line[line_len - 1] == '\n') {
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
#ifndef DEBUG
    char *homedir = getenv("HOME");
    if (homedir == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
#else
    char *homedir = ".";
#endif
    size_t len = strlen(homedir)+ sizeof SAVE_DIR_NAME + sizeof "%04zu-%02zu.txt" - 1;
    char buff[len];
    strcpy(buff, homedir);
    strcat(buff, SAVE_DIR_NAME);
    char filename[sizeof "%04zu-%02zu.txt"];
    snprintf(filename, sizeof filename, "%04zu-%02zu.txt", cal->curr_year, cal->curr_month);

    // just call mkdir, no need to check whether dir already exists.
    mkdir(buff, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    strcat(buff, filename);

    FILE *fp = fopen(buff, "w");
    if (fp == NULL) {
        printf("Error opening save file.\n");
        exit(1);
    }

    for (size_t i = 0; i < cal->cnt_dates; i++) {
        if (cal->dates[i].is_sig) {
            fprintf(fp, "%02zu;%s\n",
                    cal->dates[i].mday, cal->dates[i].note);
        }
    }
    fclose(fp);

}


// PARAMS month and year; pass 0 to get current year and month
Calendar init_calendar(size_t month, size_t year) {
    time_t t = time(NULL);
    struct tm time_now = *localtime(&t);

    Calendar cal;

    cal.curr_year      = year ? year : (time_now.tm_year + 1900);
    cal.curr_month     = month ? month : time_now.tm_mon;
    cal.curr_mday      = time_now.tm_mday;
    cal.cnt_dates      = days_in_month(cal.curr_month, cal.curr_year);
    cal.first_weekday  = first_dow_zeller(cal.curr_month, cal.curr_year);

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
        print_sig_date_note(date, &cal);
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

            Calendar cal = init_calendar(0, 0);
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
