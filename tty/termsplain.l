%option 8bit nodefault noinput nounput noyywrap

%%

\x07    { printf("\\a"); }
\x08    { printf("\\b"); }
\x09    { printf("\\t"); }
\x0d    { printf("\\r"); }
\x1b    { printf("\\e"); }
[[:print:]]+ { printf("%s", yytext); }
.       { printf("\\x%02X", yytext[0]); }

%%

#include <err.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>

int main(int argc, char *argv[])
{
    FILE *fh = NULL;
    struct timespec zzz = { .tv_sec = 0, .tv_nsec = 25000000 };

    if (argc > 2) {
        fprintf(stderr, "Usage: termsplain [filename|-]\n");
        exit(EX_USAGE);
    } else if (argc == 2) {
        if (!strncmp(argv[1], "-", (size_t) 2) == 0) {
            if ((fh = fopen(argv[1], "r")) == NULL)
                err(EX_IOERR, "could not open '%s'", argv[1]);
            yyrestart(fh);
        }
    }
    if (fh == NULL) fh = stdin;

    setvbuf(stdout, (char *) NULL, _IONBF, (size_t) 0);

    while (1) {
        yylex();
        if (ferror(fh)) err(EX_IOERR, "ferror() on input");
        nanosleep(&zzz, NULL);
        fseek(fh, 0, SEEK_CUR);
    }

    exit(1);                    /* NOTREACHED */
}
