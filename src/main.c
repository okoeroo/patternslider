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
int init_pattern(void);
int match_pattern(char const * const buf, struct pattern_s *pattern);
int fill_buffer(OFF_T os);
int doit(void);


/* Variables */
#define BUFFER_SIZE 1000 * 1000 * 10
int   fd     = -1;
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


int match_pattern(char const * const buf, struct pattern_s *pattern) {
    int i;

    for (i = 0; i < pattern->len; i++) {
        /* Ignore this byte */
        if (pattern->pattern[i] == -1) {
            continue;
        }

        /* Match buffer with pattern */
        if (buf[i] != pattern->pattern[i]) {
            return 1;
        }
    }

    return 0;
}

int siever(char const * const buf) {
    struct pattern_s *p, *tmp_p;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        if (match_pattern(buf, p) == 0) {
            printf ("Found a match for pattern %s\n", p->name);
            return 0;
        }
        tmp_p = TAILQ_NEXT(p, entries);
    }

    return 1;
}



int filters(void) {
    int i;

    for (i = 0; i < bufsize; i++) {
        if (siever(&(buffer[i])) == 0) {
            printf("Match!\n");
            continue;
        }
    }

    return 0;
}


int fill_buffer(OFF_T os)
{
    ssize_t cnt = 0;

    OFF_T ret_os;
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
        offset += bufsize;
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
