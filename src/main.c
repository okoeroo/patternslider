#include "main.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


#if 0
    #define HAVE_LSEEK64
#else
    #undef HAVE_LSEEK64
#endif

/* Prototypes */
#ifdef HAVE_LSEEK64
off64_t fill_buffer(off64_t os);
#else
off_t fill_buffer(off_t os);
#endif
int doit(void);


/* Variables */
#define BUFFER_SIZE 1000 * 1000 * 10
int   fd     = -1;
char *buffer = NULL;
#ifndef HAVE_LSEEK64
off_t offset = 0;
off_t endset = 0;
off_t bufsize = 0;
#else
off64_t offset = 0;
off64_t endset = 0;
off64_t bufsize = 0;
#endif

/* functions */
#ifdef HAVE_LSEEK64
off64_t fill_buffer(off64_t os)
#else
off_t fill_buffer(off_t os)
#endif
{
    ssize_t cnt = 0;

#ifdef HAVE_LSEEK64
    off64_t ret_os;
    os = lseek64(fd, os, SEEK_SET);
#else
    off_t ret_os;
    os = lseek(fd, os, SEEK_SET);
#endif
    cnt = read(fd, buffer, bufsize);
    printf ("Read %ld bytes into buffer.\n", cnt);
    /* display buffer */

    return ret_os;
}


int doit(void)
{
    /* Begin of file */
    offset = 0;
    offset = fill_buffer(offset);
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
#ifdef HAVE_LSEEK64
    offset = lseek64(fd, offset, SEEK_END);
#else
    offset = lseek(fd, offset, SEEK_END);
#endif
    endset = offset;
    printf("File size is: %llu\n", endset);


    /* Start analysis */
    doit();

    close(fd);

    return EXIT_CODE_GOOD;
}
