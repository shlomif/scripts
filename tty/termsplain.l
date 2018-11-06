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
#include <sysexits.h>
#include <time.h>

int main(void)
{
    struct timespec zzz = { .tv_sec = 0, .tv_nsec = 25000000 };
    setvbuf(stdout, (char *) NULL, _IONBF, (size_t) 0);
    while (1) {
        yylex();
        if (ferror(stdin)) err(EX_IOERR, "ferror() stdin");
        nanosleep(&zzz, NULL);
        fseek(stdin, 0, SEEK_CUR);
    }
    exit(1);                    /* NOTREACHED */
}
