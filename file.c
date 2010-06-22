#include "8cc.h"

void error(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "8cc: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(-1);
}

File *make_file(FILE *stream, char *filename) {
    File *r = malloc(sizeof(File));
    r->stream = stream;
    r->line = 1;
    r->filename = make_string();
    ostr(r->filename, filename);
    r->ungotten = EOF;
    return r;
}

File *open_file(char *path) {
    if (!strcmp(path, "-")) {
	return make_file(stdin, "-");
    }
    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
	perror("fopen failed: ");
	exit(-1);
    }
    return make_file(stream, path);
}

void unreadc(int c, File *file) {
    if (file->ungotten != EOF)
	ungetc(file->ungotten, file->stream);
    file->ungotten = c;
}

int readc(File *file) {
    int c;
    if (file->ungotten == EOF) {
	c = getc(file->stream);
    } else {
	c = file->ungotten;
	file->ungotten = EOF;
    }
    if (c == EOF || c == '\0') return EOF;
    if (c == '\r') {
	file->line++;
	c = getc(file->stream);
	if (c == '\n') return c;
	unreadc(c, file);
	return '\n';
    }
    if (c == '\n') file->line++;
    return c;
}

int getfileline(File *file) {
    return file->line;
}

char *getfilename(File *file) {
    return STRING_BODY(file->filename);
}
