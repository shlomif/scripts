//
//  main.m
//  ltagger - NSLinguisticTagger utility for CLI (mostly to see how "words"
//    are defined).
//
//  Copyright (c) 2014 Jeremy Mates. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <err.h>
#import <sysexits.h>

const char *Program_Name;

void emit_help(void);int main(int argc, const char * argv[]) {
    @autoreleasepool {
        int ch;
        Program_Name = *argv;
        
        while ((ch = getopt(argc, (char * const *)argv, "h?")) != -1) {
            switch (ch) {
                case 'h':
                case '?':
                default:
                    emit_help();
                    /* NOTREACHED */
            }
        }
        argc -= optind;
        argv += optind;

        char *linep = NULL;
        const char *fname;
        FILE *fh;
        size_t len = 0;
        if (!*argv || strncmp(*argv, "-", (size_t) 2) == 0) {
            fh = stdin;
            fname = "-";
        } else {
            if ((fh = fopen(*argv, "r")) == NULL)
                err(EX_IOERR, "could not open '%s'", *argv);
            fname = *argv;
        }
        while (getline(&linep, &len, fh) > 0) {
            NSString *theLine = [[NSString alloc] initWithCString:linep
                                                   encoding:NSUTF8StringEncoding];
            NSRange allOfit = NSMakeRange(0, [theLine length]);
            [theLine enumerateLinguisticTagsInRange:allOfit
                                             scheme:NSLinguisticTagSchemeTokenType
                                            options:NSLinguisticTaggerOmitPunctuation|NSLinguisticTaggerOmitWhitespace|NSLinguisticTaggerOmitOther
                                        orthography:nil
                                         usingBlock:^(NSString *tag, NSRange tokenRange, NSRange sentenceRange, BOOL *stop) {
                NSString *subStr = [theLine substringWithRange:tokenRange];
                printf("%s %s ", [tag cStringUsingEncoding:NSUTF8StringEncoding], [subStr cStringUsingEncoding:NSUTF8StringEncoding]);
            }];
            putchar('\n');
        }
        if (ferror(fh))
            err(EX_IOERR, "error reading input file %s", fname);
    }
    return 0;
}

void emit_help(void)
{
    const char *shortname;
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;
    
    fprintf(stderr, "Usage: %s [file|-]", shortname);
    exit(EX_USAGE);
}