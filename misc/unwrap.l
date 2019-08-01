/* unwrap - unfold hard-wrapped text lines */

%option 8bit nodefault noinput nounput noyywrap
%x BODY
%{
#include <err.h>
#include <sysexits.h>

int Flag_All;                   /* -a */

char fill = '\n';
int nl = 0;
%}
%%
[^\n]+  { fputs(yytext, stdout); if (Flag_All) { fill = ' '; BEGIN BODY; } }
\n\n    { putchar('\n'); nl = 1; BEGIN BODY; }
\n      { putchar('\n'); nl = 1; }

<BODY>[^\n]+ { putchar(fill); fputs(yytext, stdout); fill = ' '; nl = 0; }
<BODY>\n\n+  { putchar('\n'); fill = '\n'; nl = 1; }
<BODY>\n

<<EOF>> { if (!nl) putchar('\n'); yyterminate(); }
%%
int main(int argc, char *argv[]) {
    FILE *fh;
    int ch;
    while((ch = getopt(argc, argv, "h?a")) != -1) {
        switch (ch) {
        case 'a':
            Flag_All = 1;
            break;
        case 'h':
        case '?':
        default:
	    fprintf(stderr, "Usage: unwrap [-a] [file .. | -]\n");
	    exit(EX_USAGE);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc == 0 || strncmp(*argv, "-", (size_t) 2) == 0) {
        yylex();
    } else {
        while (*argv) {
            if ((fh = fopen(*argv, "r")) == NULL)
                err(EX_IOERR, "could not open '%s'", *argv);
            yyrestart(fh);
            yylex();
            fclose(fh);
	    argv++;
        }
    }
    exit(EXIT_SUCCESS);
}
