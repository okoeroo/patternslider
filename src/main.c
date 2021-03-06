/*
name:"JPG"              extension:"jpg" pattern:hex:"FF D8 FF E0 xx xx 4A 46 49 46 00"                end:hex:"FF D9"
name:"JPG with EXIF"    extension:"jpg" pattern:hex:"FF D8 FF E1 xx xx 45 78 69 66 00"                end:hex:"FF D9"
name:"JPG with EXIF 2"  extension:"jpg" pattern:hex:"FF D8 FF E1 FF xx xx 78 45 66 99"                end:hex:"FF D9"
name:"JPG (IFF)"        extension:"jpg" pattern:hex:"FF D8 FF E8 xx xx 53 50 49 46 46 00"             end:hex:"FF D9"
name:"JPG (Samsung)"    extension:"jpg" pattern:hex:"FF D8 FF DB"                                     end:hex:"FF D9"
name:"JPG (Canon 1D)"   extension:"jpg" pattern:hex:"FF D8 FF E2"                                     end:hex:"FF D9"
name:"JPG (Samsung D500)" extension:"jpg" pattern:hex:"FF D8 FF E3"                                     end:hex:"FF D9"
name:"PNG"              extension:"png" pattern:hex:"89 50 4E 47 0D 0A 1A 0A 00 00 00 0D 49 48 44 52" end:hex:"00 49 45 4E 44 AE 42 60 82"
name:"JPEG2000"         extension:"jp2" pattern:hex:"00 00 00 0C 6A 50 20 200D 0A"
name:"3GP"              extension:"3gp" pattern:hex:"00 00 00 nn 66 74 79 70 33 67 70"
name:"ISO Base Media file (MPEG-4) v1" extension:"mp4" pattern:hex:"00 00 00 14 66 74 79 70 69 73 6F 6D"
name:"QuickTime movie file" extension:"mov" pattern:hex:"00 00 00 14 66 74 79 70 71 74 20 20"
name:"MPEG-4 video"     extension:"mp4" pattern:hex:"00 00 00 18 66 74 79 70 33 67 70 35"
name:"MPEG-4 video/QuickTime"     extension:"m4v" pattern:hex:"00 00 00 18 66 74 79 70 6D 70 34 32"
name:"Apple Lossless Audio Codec"     extension:"m4a" pattern:hex:"00 00 00 20 66 74 79 70 4D 34 41 20"
#name:"DVD MPEG2"     extension:"vob" pattern:hex:"00 00 01 BA"
name:"Matroska"     extension:"mkv" pattern:hex:"1A 45 DF A3 93 42 82 88 6D 61 74 72 6F 73 6B 61"
name:"RealAudio"     extension:"ra" pattern:hex:"2E 72 61 FD 00"
name:"MS Money"     extension:"mny" pattern:hex:"00 01 00 00 4D 53 49 53 41 4D 20 44 61 74 61 62 61 73 65"
name:"Standard ACE"     extension:"acedb" pattern:hex:"00 01 00 00 53 74 61 6E 64 61 72 64 20 41 43 45 20 44 42"
name:"MS Jet DB"     extension:"mdb" pattern:hex:"00 01 00 00 53 74 61 6E 64 61 72 64 20 4A 65 74 20 44 42"
name:"Palm Address Book Archive"     extension:"aba" pattern:hex:"00 01 42 41"
name:"Palm Data Book"     extension:"dba" pattern:hex:"00 01 42 44"
#name:"Flash Cookies"     extension:"sol" pattern:hex:"00 BF"
#name:"MS SQL Server 2k db"     extension:"mdf" pattern:hex:"01 0F 00 00"
name:"VMware 4 Virtual Disk"     extension:"vmdk" pattern:hex:"23 20 44 69 73 6B 20 4465 73 63 72 69 70 74 6F"
name:"PDF 1"     extension:"pdf" pattern:hex:"25 50 44 46" end:hex:"0A 25 25 45 4F 46"
name:"PDF 2"     extension:"pdf" pattern:hex:"25 50 44 46" end:hex:"0A 25 25 45 4F 46 0A"
name:"PDF 3"     extension:"pdf" pattern:hex:"25 50 44 46" end:hex:"0D 0A 25 25 45 4F 46 0D 0A"
name:"PDF 4"     extension:"pdf" pattern:hex:"25 50 44 46" end:hex:"0D 25 25 45 4F 46 0D"
name:"MS Win Media Audio/Video"     extension:"wmv" pattern:hex:"30 26 B2 75 8E 66 CF 11A6 D9 00 AA 00 62 CE 6C"
name:"Photoshop"     extension:"psd" pattern:hex:"38 42 50 53"
name:"7-Zip"     extension:"7z" pattern:hex:"37 7A BC AF 27 1C"
name:"AutoCAD (var subtypes)"     extension:"dwg" pattern:hex:"41 43 31 30"
name:"vCard"     extension:"vcf" pattern:hex:"42 45 47 49 4E 3A 56 4341 52 44 0D 0A"
name:"Flash video"     extension:"flv" pattern:hex:"46 4C 56 01"
name:"GIF87a"     extension:"gif" pattern:hex:"47 49 46 38 37 61"
name:"GIF89a"     extension:"gif" pattern:hex:"47 49 46 38 39 61"
name:"Canon RAW 1"     extension:"raw" pattern:hex:"49 49 2A 00 10 00 00 0043 52"
name:"Canon RAW 2"     extension:"raw" pattern:hex:"49 49 1A 00 00 00 48 4541 50 43 43 44 52 02 00"
name:"TIF"     extension:"tif" pattern:hex:"49 49 2A 00"
name:"PGP Disk image"     extension:"pgd" pattern:hex:"50 47 50 64 4D 41 49 4E"
*/

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
#include <sys/stat.h>
#include <errno.h>
#include <sys/syslimits.h>

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
    size_t          min_bytes;
    size_t          max_bytes;
    TAILQ_ENTRY(pattern_s) entries;
};
TAILQ_HEAD(, pattern_s) pattern_head;


/* Prototypes */


/* Variables */
#define BUFFER_SIZE         1000 * 1000 * 10
#define DEFAULT_DUMP_SIZE   16000
#define MAX_DUMP_SIZE       1000 * 1000 * 10
#define DUMP_FILE_PREFIX    "dump"
FILE * output_fh;
char *dump_dir = NULL;
char *input_file = NULL;
char *output_file = NULL;
char *pattern_file = NULL;
int   longest_pattern = 0;
int   fd     = -1;
int   dump_num = 0;
unsigned char *buffer = NULL;
OFF_T offset = 0;
OFF_T endset = 0;
size_t bufsize = BUFFER_SIZE;

/* functions */

void update_info(void) {
    float x;
    float a;
    float b;

    a = offset;
    b = endset;
    x = a / b * 100;

    fprintf(stdout, "\r");
    fprintf(stdout, "File size: %llu, buffer size: %lu, current offset: %llu, percentage read: %.3f % \% %% ",
        endset,
        bufsize,
        offset,
        x);
}


void add_pattern(const char * const line) {
    char hex[3];
    char *ret_p;
    long n;
    size_t i;
    struct pattern_s *p;
    conf_section_t in_section = SECTION_NONE;

    p = malloc(sizeof(struct pattern_s));
    if (p == NULL) {
        printf("Error: Failed to allocate memory for a pattern\n");
        exit(1);
    }
    memset(p, 0, sizeof(struct pattern_s));

    for(i = 0; i < strlen(line); i++) {

        if (strncmp(&(line[i]), "name:\"", 6) == 0) {
            i += 6;
            if (in_section != SECTION_NONE) {
                printf("Error: Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_NAME;
        } else if (strncmp(&(line[i]), "extension:\"", strlen("extension:\"")) == 0) {
            i += strlen("extension:\"");
            if (in_section != SECTION_NONE) {
                printf("Error: Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_EXTENSION;
        } else if (strncmp(&(line[i]), "pattern:hex:\"", 13) == 0) {
            i += 13;
            if (in_section != SECTION_NONE) {
                printf("Error: Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_PATTERN_HEX;
        } else if (strncmp(&(line[i]), "end:hex:\"", 9) == 0) {
            i += 9;
            if (in_section != SECTION_NONE) {
                printf("Error: Parse error on line (missing closing tag): \"%s\"\n", line);
                exit(1);
            }
            in_section = SECTION_END_PATTERN_HEX;
        } else if (line[i] == '\"') {
            if (in_section == SECTION_NONE) {
                printf("Error: Parse error on line (a wild \" in the line): \"%s\"\n", line);
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
                if ((line[i] == 'x') && (line[i] == 'x')) {
                    p->pattern[p->len] = -1;
                    p->len++;
                    i++;
                } else if (ishexnumber(line[i]) && ishexnumber(line[i+1])) {
                    hex[0] = line[i];
                    hex[1] = line[i+1];

                    n = strtoul(hex, &ret_p, 16);
                    if (*ret_p != 0 ) {
                        printf("Error: Not a Hex number: %s\n", hex);
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
                        printf("Error: Not a Hex number: %s\n", hex);
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
        add_pattern(line);
    }

}

void print_stored_patterns(void) {
    struct pattern_s *p, *tmp_p;
    int i;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        fprintf(output_fh, "== Pattern == name:\"%s\"", p->name);
        if (p->len_extension) {
            fprintf(output_fh, " extension:\"%s\"", p->extension);
        }
        if (p->len) {
            fprintf(output_fh, " pattern:\"");
            for (i = 0; i < p->len; i++) {
                fprintf(output_fh, "%x ", p->pattern[i]);
            }
            fprintf(output_fh, "\b\"");
        }
        if (p->end_len) {
            fprintf(output_fh, " end pattern:\"");
            for (i = 0; i < p->end_len; i++) {
                fprintf(output_fh, "%x", p->end_pattern[i]);
                if (i < p->end_len) {
                    fprintf(output_fh, " ");
                }
            }
            fprintf(output_fh, "\"");
        }
        fprintf(output_fh, "\n");
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
        printf("Error: Failed to read config file.\n");
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

int match_pattern(unsigned char const * const buf, unsigned short *pattern, int len) {
    int i;

    for (i = 0; i < len; i++) {
        /* Ignore this byte */
        if (pattern[i] == (unsigned short) -1) {
            continue;
        }

        /* Match buffer with pattern */
        if (buf[i] != pattern[i]) {
            return 1;
        }
    }
    return 0;
}

void dump_buffer(unsigned char const * const buf, OFF_T os, OFF_T len, struct pattern_s *p) {
    int dump_fd = -1;
    char dumpfile[PATH_MAX];
    OFF_T dump_size;

    if (len > MAX_DUMP_SIZE) {
        dump_size = MAX_DUMP_SIZE;
    } else {
        dump_size = len;
    }

    snprintf(dumpfile, PATH_MAX, "%s/%s_%08d.%s",
                                dump_dir ? dump_dir : "/tmp/dump",
                                DUMP_FILE_PREFIX,
                                dump_num,
                                p->len_extension ? p->extension : "jpg");
    dump_num++;

    dump_fd = open(dumpfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (dump_fd < 1) {
        return;
    }
    write(dump_fd, &(buf[os]), dump_size);
    close(dump_fd);
    fprintf(output_fh, "Dumped file %s (size:%lld bytes, pattern:%s)\n", dumpfile, dump_size, p->name);
    fflush(output_fh);
}

size_t sieve_end_pattern(unsigned char const * const buf, size_t os, struct pattern_s *p) {
    size_t i;

    for (i = 0; i < (bufsize - os); i++) {
        if (match_pattern(&(buf[i]), p->end_pattern, p->end_len) == 0) {
            return (i + p->end_len) < bufsize ? i + p->end_len : bufsize;
        }
    }
    return 0;
}

int siever(unsigned char const * const buf, size_t os) {
    struct pattern_s *p, *tmp_p;
    size_t ret;

    for (p = TAILQ_FIRST(&pattern_head); p != NULL; p = tmp_p) {
        if (match_pattern(buf, p->pattern, p->len) == 0) {
            if (p->end_len) {
                ret = sieve_end_pattern(buf, os, p);
                if (ret == 0) {
                    dump_buffer(buf, 0, DEFAULT_DUMP_SIZE, p);
                } else {
                    dump_buffer(buf, 0, ret, p);
                }
            } else {
                dump_buffer(buf, 0, DEFAULT_DUMP_SIZE, p);
            }
            return 0;
        }
        tmp_p = TAILQ_NEXT(p, entries);
    }

    return 1;
}

int filters(void) {
    size_t i;
    int rotor = 0;

    for (i = 0; i < bufsize; i++) {
        if (siever(&(buffer[i]), i) == 0) {
            continue;
        }
        if (i % 1000000 == 0) {

            switch(rotor++) {
                case 0:
                    printf("\b\b \\");
                    break;
                case 1:
                    printf("\b\b |");
                    break;
                case 2:
                    printf("\b\b /");
                    break;
                case 3:
                    printf("\b\b -");
                    rotor = 0;
                    break;
            }
            fflush(stdout);
        }
    }

    return 0;
}

int fill_buffer(OFF_T os)
{
    ssize_t cnt = 0;
    os = LSEEK(fd, os, SEEK_SET);

    cnt = read(fd, buffer, bufsize);
    fprintf (output_fh, "Read %ld bytes into buffer.\n", cnt);

    return 0;
}

int doit(void)
{
    printf("\n");
    fflush(stdout);

    while (offset < endset) {
        /* Fill buffer */
        if (fill_buffer(offset)) {
            printf("Something went wrong at offset: %lld\n", offset);
            return 1;
        }

        /* Status line print */
        update_info();

        /* Run filters on buffer */
        filters();

        /* Refill buffer */
        offset += bufsize; /* forward the file read-in by the size of the buffer */
        offset -= longest_pattern; /* compensate for the size of the longest pattern to match on */
    }
    return 0;
}

int check_dump_dir(void) {
    struct stat s;

    if (stat(dump_dir ? dump_dir : "/tmp/dump", &s) < 0) {
        printf("Error: the dump directory \"%s\" is unreachable, reason: %s\n", dump_dir ? dump_dir : "/tmp/dump", strerror(errno));
        return 1;
    }
    return 0;
}

void usage(void) {
    printf("patternslider \\\n\t-m <buffer size: 100{,k,M,G,T,P,E,Z,Y}> \\\n\t-p <patterns> \\\n\t-d <dump dir> \\\n\t-i <input blob> \\\n\t-offset <offset absolute or percentage> \\\n\t-o <output file> \\\n\t[-h]\n");
}

int main(int argc, char * argv[])
{
    int      i;
    char *rest;
    long long retll = 0;
    short percentage = 0;

    float x;
    float a;
    float b;

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
        } else if (strcmp("-offset", argv[i]) == 0) {
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
            if (strstr(argv[i+1], "%") == NULL) {
                /* Absolute offset */
                rest = NULL;
                retll = strtoll(argv[i+1], &rest, 10);
                percentage = 0;
            } else {
                /* Percentage */
                rest = NULL;
                retll = strtoll(argv[i+1], &rest, 10);
                percentage = 1;
            }
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
            rest = NULL;
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

    /* Open output file, if configured */
    if (output_file) {
        output_fh = fopen(output_file, "a");
        if (!output_fh) {
            printf("Error: could not open outputfile \"%s\" for writing\n", output_file);
            exit(1);
        }
    } else {
        output_fh = stdout;
    }

    /* Can I dump the files */
    if (check_dump_dir() != 0) {
        exit(1);
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
    fprintf(output_fh, "File size is: %llu\n", endset);

    /* Begin of file */
    offset = 0;
    if (percentage == 0) {
        offset = retll;
    } else {
        a = retll;
        b = endset;
        x = a / 100 * b;
        offset = x;
    }

    printf("Offset to start from is: %llu / %llu\n", offset, endset);
    fprintf(output_fh, "Offset to start from is: %llu / %llu\n", offset, endset);

    /* Start analysis */
    doit();

    close(fd);

    printf("\n");
    return EXIT_CODE_GOOD;
}
