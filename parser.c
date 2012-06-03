/*
name:"JPG" extension:"jpg" pattern:hex:"FF A0 B0" end:hex:"FF 00"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <queue.h>
#include <ctype.h>


typedef enum {
    SECTION_NONE, SECTION_EXTENSION, SECTION_NAME, SECTION_PATTERN_HEX, SECTION_END_PATTERN_HEX
} conf_section_t;

struct pattern_s {
    char            name[255];
    int             len_name;
    char            extension[255];
    int             len_extension;
    unsigned short  pattern[255];
    int             len;
    unsigned short  end_pattern[255];
    int             end_len;
    TAILQ_ENTRY(pattern_s) entries;
};
TAILQ_HEAD(, pattern_s) pattern_head;


void add_pattern(const char * const line) {
    char hex[3];
    char *ret_p;
    long n;
    off_t i;
    struct pattern_s *p;
    conf_section_t in_section = SECTION_NONE;

    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        exit(1);
    }
    memset(p, 0, sizeof(struct pattern_s));

    for(i = 0; i < strlen(line); i++) {

        if (strncmp(&(line[i]), "name:\"", 6) == 0) {
            i += 6;
            if (in_section != SECTION_NONE) {
                printf("Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_NAME;
        } else if (strncmp(&(line[i]), "extension:\"", strlen("extension:\"")) == 0) {
            i += strlen("extension:\"");
            if (in_section != SECTION_NONE) {
                printf("Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_EXTENSION;
        } else if (strncmp(&(line[i]), "pattern:hex:\"", 13) == 0) {
            i += 13;
            if (in_section != SECTION_NONE) {
                printf("Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_PATTERN_HEX;
        } else if (strncmp(&(line[i]), "end:hex:\"", 9) == 0) {
            i += 9;
            if (in_section != SECTION_NONE) {
                printf("Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_END_PATTERN_HEX;
        } else if (line[i] == '\"') {
            if (in_section == SECTION_NONE) {
                printf("Parse error on line (a wild \" in the line): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_NONE;
        }

        switch(in_section) {
            case SECTION_NAME            :
                p->name[p->len_name++] = line[i];
                break;
            case SECTION_EXTENSION       :
                p->extension[p->len_extension++] = line[i];
                break;
            case SECTION_PATTERN_HEX     :
                hex[0] = '\0'; hex[1] = '\0'; hex[2] = '\0';

                if (isblank(line[i])) {
                    continue; /* Skip byte on blank */
                }
                if (ishexnumber(line[i]) && ishexnumber(line[i+1])) {
                    hex[0] = line[i];
                    hex[1] = line[i+1];

                    n = strtoul(hex, &ret_p, 16);
                    if (*ret_p != 0 ) {
                        printf("Not a Hex number: %s\n", hex);
                        exit(1);
                    }
                    p->pattern[p->len] = n;
                    p->len++;

                    i++;
                }
                break;
            case SECTION_END_PATTERN_HEX :
                hex[0] = '\0'; hex[1] = '\0'; hex[2] = '\0';

                if (isblank(line[i])) {
                    continue; /* Skip byte on blank */
                }
                if (ishexnumber(line[i]) && ishexnumber(line[i+1])) {
                    hex[0] = line[i];
                    hex[1] = line[i+1];

                    n = strtoul(hex, &ret_p, 16);
                    if (*ret_p != 0 ) {
                        printf("Not a Hex number: %s\n", hex);
                        exit(1);
                    }
                    p->end_pattern[p->end_len] = n;
                    p->end_len++;

                    i++;
                }
                break;
            case SECTION_NONE:
                break;
        }
    }
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);
}

void getaline(const char * const buf, off_t bol, off_t eol) {
    char * line;

    line = malloc(eol - bol + 1);
    snprintf(line, eol - bol + 1, "%s", &(buf[bol]));

    /*
    printf("--> \"%s\"\n", line);
    printf("eol - bol = %lld\n", eol - bol);
    */

    if (strncmp(line, "name:", 5) == 0) {
        printf("=============== %s\n", line);
        add_pattern(line);
    }

}


void print_stored_patterns(void) {
    struct pattern_s *p, *tmp_p;
    int i;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        printf("Pattern name:\"%s\"", p->name);
        if (p->len_extension) {
            printf(" extension:\"%s\"", p->extension);
        }
        if (p->len) {
            printf(" pattern:\"");
            for (i = 0; i < p->len; i++) {
                printf("%x ", p->pattern[i]);
            }
            printf("\b\"");
        }
        if (p->end_len) {
            printf(" end pattern:\"");
            for (i = 0; i < p->end_len; i++) {
                printf("%x ", p->end_pattern[i]);
            }
            printf("\b\"");
        }
        printf("\n");
        tmp_p = TAILQ_NEXT(p, entries);
    }
}


int init_patterns(char *conf) {
    off_t i, cnt, os, es, eol, bol;
    int fdc = -1;
    char *buf;
    struct pattern_s *p;

    /* Initialize the tail queue. */
    TAILQ_INIT(&pattern_head);

    fdc = open(conf, O_RDONLY);
    if (fdc < 0) {
        return 1;
    }
    os = 0;
    es = lseek(fdc, os, SEEK_END);
    os = lseek(fdc, 0, SEEK_SET);

    buf = malloc(es);
    cnt = read(fdc, buf, es);
    if (cnt != es) {
        printf("Failed to read config file.\n");
    }
    close(fdc);

    bol = 0;
    for (i = 0; i < es; i++){
        /* Search end of line */
        if (buf[i] == '\n') {
            eol = i;

            /* printf("- pos: %d, bol: %d, eol: %d  %c\n", i, bol, eol, buf[bol]); */

            getaline(buf, bol, eol);
            bol = i + 1;
        }
    }
    print_stored_patterns();

    return 0;
}

int main(void) {
    return initialize_patterns("parser.c");
}

