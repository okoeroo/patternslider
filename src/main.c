#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>

#include <queue.h>


#if 0
    #define HAVE_LSEEK64
    #define HAVE_OFF64_t
#else
    #undef HAVE_LSEEK64
    #undef HAVE_OFF64_t
#endif


#ifdef HAVE_OFF64_t
    #define OFF_T off64_t
#else
    #define OFF_T off_t
#endif

#ifdef HAVE_LSEEK64
    #define LSEEK LSEEK64
#else
    #define LSEEK lseek
#endif


typedef enum {
    SECTION_NONE, SECTION_EXTENSION, SECTION_NAME,
    SECTION_PATTERN_HEX, SECTION_END_PATTERN_HEX
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


/* Prototypes */


/* Variables */
#define BUFFER_SIZE 1000 * 1000 * 10
char *dump_dir = NULL;
char *input_file = NULL;
char *output_file = NULL;
char *pattern_file = NULL;
int   longest_pattern = 0;
int   fd     = -1;
int   dump_num = 0;
unsigned char *buffer = NULL;
#ifndef HAVE_LSEEK64
OFF_T offset = 0;
OFF_T endset = 0;
size_t bufsize = BUFFER_SIZE;
#else
OFF_T offset = 0;
OFF_T endset = 0;
OFF_T bufsize = 0;
#endif

/* functions */
/*
name:"JPG" extension:"jpg" pattern:hex:"FF A0 B0" end:hex:"FF 00"
*/


void add_pattern(const char * const line) {
    char hex[3];
    char *ret_p;
    long n;
    size_t i;
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

void getaline(const char * const buf, OFF_T bol, OFF_T eol) {
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
    OFF_T i, cnt, os, es, eol, bol;
    int fdc = -1;
    char *buf;

    /* Initialize the tail queue. */
    TAILQ_INIT(&pattern_head);

    fdc = open(conf, O_RDONLY);
    if (fdc < 0) {
        return 1;
    }
    os = 0;
    es = LSEEK(fdc, os, SEEK_END);
    os = LSEEK(fdc, 0, SEEK_SET);

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

#if 0
int
init_pattern(void) {
    struct pattern_s *p;

    /* Initialize the tail queue. */
    TAILQ_INIT(&pattern_head);

#if 0
    /* HTTP */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    p->name        = "HTTP";
    p->len         = 4;
    p->pattern[0]  = 'H';
    p->pattern[1]  = 'T';
    p->pattern[2]  = 'T';
    p->pattern[3]  = 'P';
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);
#endif

    /* JPG */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    memset(p, 0, sizeof(struct pattern_s));
    p->name        = "JPG";
    p->len         = 11;
    p->pattern[0]  = 0xFF;
    p->pattern[1]  = 0xD8;
    p->pattern[2]  = 0xFF;
    p->pattern[3]  = 0xE0;
    p->pattern[4]  = -1;
    p->pattern[5]  = -1;
    p->pattern[6]  = 0x4A;
    p->pattern[7]  = 0x46;
    p->pattern[8]  = 0x49;
    p->pattern[9]  = 0x46;
    p->pattern[10] = 0x00;

    p->end_len        = 2;
    p->end_pattern[0] = 0xFF;
    p->end_pattern[1] = 0xD9;
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);

    /* JPG with EXIF */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    memset(p, 0, sizeof(struct pattern_s));
    p->name        = "JPG with EXIF";
    p->len         = 11;
    p->pattern[0]  = 0xFF;
    p->pattern[1]  = 0xD8;
    p->pattern[2]  = 0xFF;
    p->pattern[3]  = 0xE1;
    p->pattern[4]  = -1;
    p->pattern[5]  = -1;
    p->pattern[6]  = 0x45;
    p->pattern[7]  = 0x78;
    p->pattern[8]  = 0x69;
    p->pattern[9]  = 0x66;
    p->pattern[10] = 0x00;
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);

    /* JPG with EXIF_2 */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    memset(p, 0, sizeof(struct pattern_s));
    p->name        = "JPG with EXIF_2";
    p->len         = 11;
    p->pattern[0]  = 0xFF;
    p->pattern[1]  = 0xD8;
    p->pattern[2]  = 0xFF;
    p->pattern[3]  = 0xE1;
    p->pattern[4]  = 0xFF;
    p->pattern[5]  = -1;
    p->pattern[6]  = -1;
    p->pattern[7]  = 0x78;
    p->pattern[8]  = 0x45;
    p->pattern[9]  = 0x66;
    p->pattern[10] = 0x99;
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);

    /* JPG (IFF) */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    memset(p, 0, sizeof(struct pattern_s));
    p->name        = "JPG (IFF)";
    p->len         = 12;
    p->pattern[0]  = 0xFF;
    p->pattern[1]  = 0xD8;
    p->pattern[2]  = 0xFF;
    p->pattern[3]  = 0xE8;
    p->pattern[4]  = -1;
    p->pattern[5]  = -1;
    p->pattern[6]  = 0x53;
    p->pattern[7]  = 0x50;
    p->pattern[8]  = 0x49;
    p->pattern[9]  = 0x46;
    p->pattern[10] = 0x46;
    p->pattern[11] = 0x00;
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);

    /* CFLAGS */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
    memset(p, 0, sizeof(struct pattern_s));
    p->name        = "CFLAGS";
    p->len         = 6;
    p->pattern[0]  = 'C';
    p->pattern[1]  = 'F';
    p->pattern[2]  = 'L';
    p->pattern[3]  = 'A';
    p->pattern[4]  = 'G';
    p->pattern[5]  = 'S';
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);


    return 0;
}
#endif

int calc_longest_pattern(void) {
    struct pattern_s *p, *tmp_p;
    int longest = 0;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        if (longest < p->len) {
            longest = p->len;
        }
        tmp_p = TAILQ_NEXT(p, entries);
    }
    return longest;
}

int match_pattern(unsigned char const * const buf, struct pattern_s *pattern) {
    int i;

    for (i = 0; i < pattern->len; i++) {
        /* Ignore this byte */
        if (pattern->pattern[i] == (unsigned short) -1) {
            continue;
        }

        /* Match buffer with pattern */
        if (buf[i] != pattern->pattern[i]) {
            return 1;
        }
    }
    return 0;
}

void dump_buffer(unsigned char const * const buf, OFF_T os, OFF_T len) {
    int dump_fd = -1;
    char *dumpfile;

    dumpfile = malloc(255);
    if (!dumpfile) {
        exit(1);
    }

    snprintf(dumpfile, 255, "%s%d.jpg", "/tmp/dump/dump", dump_num);
    dump_num++;

    dump_fd = open(dumpfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (dump_fd < 1) {
        return;
    }


    write(dump_fd, &(buf[os]), len);
    free(dumpfile);
    close(dump_fd);
}

int siever(unsigned char const * const buf, OFF_T os) {
    struct pattern_s *p, *tmp_p;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        if (match_pattern(buf, p) == 0) {
            printf ("Found a match for pattern %s\n", p->name);
            dump_buffer(buf, 0, 8192);
            return 0;
        }
        tmp_p = TAILQ_NEXT(p, entries);
    }

    return 1;
}

int filters(void) {
    size_t i;

    for (i = 0; i < bufsize; i++) {
        if (siever(&(buffer[i]), i) == 0) {
            continue;
        }
    }

    return 0;
}

int fill_buffer(OFF_T os)
{
    ssize_t cnt = 0;
    os = LSEEK(fd, os, SEEK_SET);

    cnt = read(fd, buffer, bufsize);
    printf ("Read %ld bytes into buffer.\n", cnt);
    /* display buffer */

    return 0;
}

int doit(void)
{
    /* Begin of file */
    offset = 0;

    while (offset < endset) {
        /* Fill buffer */
        if (fill_buffer(offset)) {
            printf("Something went wrong at offset: %lld\n", offset);
            return 1;
        }

        /* Run filters on buffer */
        filters();


        /* Refill buffer */
        offset += bufsize; /* forward the file read-in by the size of the buffer */
        offset -= longest_pattern; /* compensate for the size of the longest pattern to match on */
    }
    return 0;
}

void usage(void) {
    printf("patternslider \\\n\t-m <buffer size: 100{,k,M,G,T,P,E,Z,Y}> \\\n\t-p <patterns> \\\n\t-d <dump dir> \\\n\t-i <input blob> \\\n\t-o <output file> \\\n\t[-h]\n");
}

int main(int argc, char * argv[])
{
    int      i;
    char *rest;

    /* CLI arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp("-h", argv[i]) == 0) {
            usage();
        } else if (strcmp("-d", argv[i]) == 0) {
            if ((i + 1) >= argc) {
                printf("Too few arguments\n");
                usage();
                exit(1);
            }
            if (argv[i+1][0] == '-') {
                printf("Found a flag, not a directory: \"%s\"\n", argv[i+1]);
                usage();
                exit(1);
            }
            dump_dir = argv[i+1];
            i++;
        } else if (strcmp("-m", argv[i]) == 0) {
            if ((i + 1) >= argc) {
                printf("Too few arguments\n");
                usage();
                exit(1);
            }
            if (argv[i+1][0] == '-') {
                printf("Found a flag, size of memory: \"%s\"\n", argv[i+1]);
                usage();
                exit(1);
            }
            bufsize = strtol(argv[i+1], &rest, 10);
            switch (rest[0]) {
                case 'k' :
                    bufsize *= 1000;
                    break;
                case 'M' :
                    bufsize *= 1000 * 1000;
                    break;
                case 'G' :
                    bufsize *= 1000 * 1000 * 1000;
                    break;
                case 'T' :
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    break;
                case 'P' :
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000 * 1000;
                    break;
                case 'E' :
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    break;
                case 'Z' :
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000 * 1000;
                    printf("Warning: A Zetta byte does not fit in a 64 bit number.\n");
                    break;
                case 'Y' :
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    bufsize *= 1000 * 1000;
                    printf("Warning: A Yotta byte does not fit in a 64 bit number.\n");
                    break;
                default :
                    printf("Error: %c is not an SI metric prefix. Info: http://en.wikipedia.org/wiki/Metric_prefix\n", rest[0]);
                    usage();
                    exit(1);
            }
            printf("you specified the allocation of %lu bytes for the buffer\n", bufsize);
            i++;
        } else if (strcmp("-o", argv[i]) == 0) {
            if ((i + 1) >= argc) {
                printf("Too few arguments\n");
                usage();
                exit(1);
            }
            if (argv[i+1][0] == '-') {
                printf("Found a flag, not a file path: \"%s\"\n", argv[i+1]);
                usage();
                exit(1);
            }
            output_file = argv[i+1];
            i++;
        } else if (strcmp("-i", argv[i]) == 0) {
            if ((i + 1) >= argc) {
                printf("Too few arguments\n");
                usage();
                exit(1);
            }
            if (argv[i+1][0] == '-') {
                printf("Found a flag, not a file path: \"%s\"\n", argv[i+1]);
                usage();
                exit(1);
            }
            input_file = argv[i+1];
            i++;
        } else if (strcmp("-p", argv[i]) == 0) {
            if ((i + 1) >= argc) {
                printf("Too few arguments\n");
                usage();
                exit(1);
            }
            if (argv[i+1][0] == '-') {
                printf("Found a flag, not a pattern file: \"%s\"\n", argv[i+1]);
                usage();
                exit(1);
            }
            pattern_file = argv[i+1];
            i++;
        } else {
            printf("Error: Unknown option: \"%s\"\n", argv[i]);
            exit(1);
        }
    }

    /* allocate the buffer */
    printf("Allocating buffer of %lu bytes\n", bufsize);
    buffer = malloc(bufsize);
    if (buffer == NULL) {
        printf("Could not allocate %lu\n", bufsize);
        return 1;
    }

    /* conf file read */
    if (init_patterns(pattern_file) != 0) {
        printf("Problem initializing pattern list\n");
        return 1;
    }
    longest_pattern = calc_longest_pattern();
    if (longest_pattern == 0) {
        printf("No pattern to search!\n");
        usage();
        exit(1);
    }

    if (input_file == NULL) {
        printf("No input file.\n");
        return 1;
    } else {
        printf("Using file: %s\n", input_file);
    }

    fd = open(input_file, O_RDONLY);
    if (fd < 0) {
        printf ("Couldn't open file: %s\n", input_file);
        return EXIT_CODE_BAD;
    }

    offset = 0;
    offset = LSEEK(fd, offset, SEEK_END);
    endset = offset;
    printf("File size is: %llu\n", endset);


    /* Start analysis */
    doit();

    close(fd);

    return EXIT_CODE_GOOD;
}
