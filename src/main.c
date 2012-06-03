#include "main.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

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
    #define LSEEK lseek64
#else
    #define LSEEK lseek
#endif


struct pattern_s {
    char           *name;
    unsigned short  pattern[255];
    int             len;
    TAILQ_ENTRY(pattern_s) entries;
};
TAILQ_HEAD(, pattern_s) pattern_head;


/* Prototypes */


/* Variables */
#define BUFFER_SIZE 1000 * 1000 * 10
char *dump_dir = NULL;
int   longest_pattern = 0;
int   fd     = -1;
int   dump_num = 0;
unsigned char *buffer = NULL;
#ifndef HAVE_LSEEK64
OFF_T offset = 0;
OFF_T endset = 0;
OFF_T bufsize = 0;
#else
OFF_T offset = 0;
OFF_T endset = 0;
OFF_T bufsize = 0;
#endif

/* functions */
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
    TAILQ_INSERT_TAIL(&pattern_head, p, entries);

    /* JPG with EXIF */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
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

    /* JPG with EXIF */
    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Failed to allocate memory for a pattern\n");
        return 1;
    }
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
    OFF_T i;
    int fd = -1;
    char *dumpfile;

    dumpfile = malloc(255);
    if (!dumpfile) {
        exit(1);
    }

    snprintf(dumpfile, 255, "%s%d.jpg", "/tmp/dump/dump", dump_num);
    dump_num++;

    fd = open(dumpfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 1) {
        return;
    }


    write(fd, &(buf[os]), len);
    free(dumpfile);
    close(fd);
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
    OFF_T i;

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

int main(int argc, char * argv[])
{
    char    *filename;

    printf("Allocate buffer of 10M\n");
    bufsize = BUFFER_SIZE;
    buffer = malloc(bufsize);
    if (buffer == NULL) {
        printf("Could not allocate %lld\n", bufsize);
        return 1;
    }
    if (init_pattern() != 0) {
        printf("Problem initializing pattern list\n");
        return 1;
    }
    longest_pattern = calc_longest_pattern();

    /* dump_dir */
    if (argc == 1) {
        printf("No input file.\n");
        return 1;
    } else {
        filename = argv[1];
        printf("Using file: %s\n", filename);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf ("Couldn't open file: %s\n", filename);
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
