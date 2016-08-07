#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <stdio.h>
#include "bstrlib.h"
#include "vector.h"

typedef struct {
    bstring key;
    bstring value;
} XMLAtribut;

typedef struct {
    bstring tagname;
    Vector *atribut;
    bstring text;
    Vector *downtags; 
} XMLTag;

/*typedef struct {
    int pos;
    bstring str;
} XMLfile; */

bstring bgetline(FILE *stream);
bstring xml_filetostr(FILE *xmlsrc);

int xml_tagnamesearch(XMLTag *elem);
void xml_tabprint(int tabs, FILE *stream, const char *fmt, ...);
void xml_treego(XMLTag *root, FILE *stream, int (*search)(XMLTag *elem));

XMLTag *xml_parse(FILE *xmlfile);
void xml_freetree(XMLTag *root);

#endif
