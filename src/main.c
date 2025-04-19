#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


#define RESET       "\033[0m"
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */

#define MAX_DATES_IN_MONTH 31
#define MAX_NOTE_LEN 255
#define MAX_LINE_LEN sizeof("xx.xx.xx;") + MAX_NOTE_LEN

// This will be concatenated to home dir except for debug
#define SAVE_FILE_NAME "/.alma.txt"

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
    char *note;
} Date;

typedef struct {
    size_t first_weekday;
    size_t curr_month;
    size_t curr_year;
    size_t curr_date;
    size_t cnt_dates;
    Date dates[MAX_DATES_IN_MONTH];
} Calendar; 

// current month's first day to next month's first day
size_t days_in_month(struct tm *time_now) {

    struct tm next_month = { 0 };
    next_month.tm_year = time_now->tm_year;
    next_month.tm_mon = time_now->tm_mon + 1;
    next_month.tm_mday = 1;
    next_month.tm_isdst = -1;

    struct tm this_month = { 0 };
    this_month.tm_year = time_now->tm_year;
    this_month.tm_mon = time_now->tm_mon;
    this_month.tm_mday = 1;
    this_month.tm_isdst = -1;

    time_t next_month_t = mktime(&next_month);
    time_t this_month_t = mktime(&this_month);

    double dDays = difftime(next_month_t, this_month_t) / 86400;
    size_t days = (size_t) dDays;
    return days;
}

// Get first weekday of a month
size_t get_first_weekday(struct tm *time_now) {
    struct tm this_month = { 0 };
    this_month.tm_year = time_now->tm_year;
    this_month.tm_mon = time_now->tm_mon;
    this_month.tm_mday = 1;
    this_month.tm_isdst = -1;

    mktime(&this_month);
    return this_month.tm_wday;
}

// Split string by a delimeter, results saved to linedata
void split_by_delim(char *src, char *delim, char **linedata, size_t size) {
    char *tok = strtok(src, delim);
    size_t sz = 0;

    while (tok != NULL && sz < size) {
        linedata[sz++] = tok;     
        tok = strtok(NULL, delim);
    }
}

void warn_date_doesnt_exist(Calendar *cal) {
    printf("Date doesn't exist.\n");
    printf("%s has only %zu days.\n", MONTHS[cal->curr_month], cal->cnt_dates);
}

void add_sig_date(Calendar *cal, size_t date, char *note) {
    if (date > cal->cnt_dates || date < 1) {
        return;
    }
    else {
        cal->dates[date - 1].is_sig = true;
        cal->dates[date - 1].note = (char*) malloc(strlen(note) + 1);
        strcpy(cal->dates[date - 1].note, note);
    }
}

void remove_sig_date(Calendar *cal, size_t date) {
    if (date > cal->cnt_dates) {
        warn_date_doesnt_exist(cal);
    }
    else {
        cal->dates[date - 1].is_sig = false;
        free(cal->dates[date - 1].note);
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

    if (calendar->dates[calendar->curr_date - 1].is_sig) {
        printf("You have a note for today: %s\n", calendar->dates[calendar->curr_date-1].note);
    }

} 

void populate_dates(Calendar *cal) {
    Date *date = NULL;
    for (size_t i = 0; i < cal->cnt_dates; i++) {
        date = &cal->dates[i];
        date->mday = i + 1;
        date->is_current = (date->mday == cal->curr_date);
        date->is_sig = false;
    }
}

void parse_sig_date(Calendar *cal, char *line) {
    char *linedata[2];
    char *datedata[3];
    split_by_delim(line, ";", linedata, 2);
    split_by_delim(linedata[0], ".", datedata, 3);
    size_t date_mday = (size_t) strtol(datedata[0], (char **)NULL, 10);
    add_sig_date(cal, date_mday, linedata[1]);
}

void slurp_sig_dates(Calendar *cal) {
#ifndef DEBUG
    char *homedir = getenv("HOME");
#else
    char *homedir = ".";
#endif
    size_t len = strlen(homedir)+ sizeof SAVE_FILE_NAME + 1;
    char buff[len];
    strcpy(buff, homedir);
    strcat(buff, SAVE_FILE_NAME);
    FILE* file = fopen(buff, "r");

    // Nothing to read, return
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
        parse_sig_date(cal, line);
    }

    fclose(file);
}

void save_new_sig_dates(Calendar *cal) {
#ifndef DEBUG
    const char *homedir = getenv("HOME");
#else
    const char *homedir = ".";
#endif
    size_t len = strlen(homedir)+ sizeof SAVE_FILE_NAME + 1;
    char buff[len];
    strcpy(buff, homedir);
    strcat(buff, SAVE_FILE_NAME);
    
    FILE *fp = fopen(buff, "w");
    if (fp == NULL) {
        printf("Error opening save file.");
        exit(1);
    }

    for (size_t i = 0; i < cal->cnt_dates; i++) {
        if (cal->dates[i].is_sig) {
            fprintf(fp, "%02zu.%02zu.%zu;%s\n",
                    cal->dates[i].mday, cal->curr_month + 1, cal->curr_year, cal->dates[i].note);
        }
    }
    fclose(fp);

}

Calendar init_calendar(void) {
    time_t t = time(NULL);
    struct tm time_now = *localtime(&t);

    Calendar cal;

    cal.curr_year      = time_now.tm_year + 1900;
    cal.curr_month     = time_now.tm_mon;
    cal.curr_date      = time_now.tm_mday;
    cal.cnt_dates      = days_in_month(&time_now);
    cal.first_weekday  = get_first_weekday(&time_now);

    populate_dates(&cal);
    slurp_sig_dates(&cal);

    return cal;
}          

void free_notes(Calendar* cal) {
    for(size_t i = 0; cal->cnt_dates; i++) {
        if (cal->dates[i].is_sig) {
            free(cal->dates[i].note);
        }
    }
}

int main(int argc, char **argv) {

    Calendar cal = init_calendar();

    // Show the note of a date
    if (argc == 2) {

        char *endptr;
        int date = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
            free_notes(&cal);
            printf("Invalid input, please enter an integer for date.\n");
            return 1;
        }

        print_sig_date_note(date, &cal);
    }

    // Add a note to a date
    else if (argc > 2 && strncmp("sig", argv[1], 3) == 0) {

        char *endptr;
        int date = strtol(argv[2], &endptr, 10);

        if (*endptr != '\0') {
            free_notes(&cal);
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

            add_sig_date(&cal, date, note);
            save_new_sig_dates(&cal);
        }
        else {
            printf("Error reading note.\n");
            free_notes(&cal);
            return 1;
        }
    }

    // Remove note of a date 
    else if (argc > 2 && strncmp("rm", argv[1], 2) == 0) {
        char *endptr;
        int date = strtol(argv[2], &endptr, 10);

        if (*endptr != '\0') {
            printf("Invalid input, please enter an integer for date.\n");
            free_notes(&cal);
            return 1;
        }
        remove_sig_date(&cal, date);
        save_new_sig_dates(&cal);
        printf("Note removed.\n");
    }

    else {
        print_calendar(&cal);
    }

    free_notes(&cal);

    return 0;
} 
