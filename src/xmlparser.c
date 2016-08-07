/* 
 * xmlparser.c 
 * Nástroje na tvorbu Abstraktného syntaktického stromu (AST)
 * základného formátu XML(1.0, UTF-8). API je zatiaľ veľmi primitívne
 *
 * Autor: Miroslav Hájek
 * Vytvorené: 2.8.2016
 * Posledná úprava: 8.8.2016
 * Licencia: MIT / LGPLv2
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "xmlparser.h"
#include "bstrlib.h"
#include "vector.h"

/* Globálne informácie o xml súbore zaregistrované v xml_parse
   tj. prakticky sa zatiaľ nedá podporovať multithreading */
static long g_filepos;
static bstring g_xmltext;

#define istag_closing(TAGNAME)      \
    (bchar((TAGNAME), 0) == '/' ? 1 : 0)

/* Lokálne funkcie - prototypy */
static bstring xml_getlextoken(char teminator);
static bstring xml_gettag(void);
static Vector *xml_atributelist(void);
static bstring xml_tagtext(void);
static XMLTag *xml_buildtree(void);
static XMLTag *xml_taginit(void); 
static void delete_tag(void *item);
static void delete_xmlatrib(void *data);
static void print_error(const char *fmt, ...);

bstring bgetline(FILE *stream) 
{
    bstring b = bgets((bNgetc)fgetc, stream,'\n');
    if (blength(b) > 0)
        b->data[--b->slen] = '\0';
    return b;
}

bstring xml_filetostr(FILE *xmlsrc)
{   
    bstring tmp;
    bstring strxml = bfromcstr("");

    /* musíme zahrnúť aj '\n' pretože by zlyhal na prázdnych riadkach*/
    while ((tmp = bgets((bNgetc)fgetc, xmlsrc, '\n')) != NULL) {
        btrimws(tmp); 
        bconcat(strxml, tmp);
        bdestroy(tmp);
    }
    
    return strxml;
}

XMLTag *xml_parse(FILE *xmlfile) 
{
    XMLTag *tg;
    g_filepos = 0;

    g_xmltext = xml_filetostr(xmlfile);
    tg = xml_buildtree();

    bdestroy(g_xmltext);
    return tg;
}

void xml_tabprint(int tabs, FILE *stream, const char *fmt, ...)
{
    va_list argum;
    int i;
    for (i = 0; i < tabs; i++) {
        fprintf(stream,"|   ");
    }
    if (strstr(fmt, "Element") == NULL) 
        fputs("   ", stream);
    va_start(argum, fmt);
    vfprintf(stream, fmt, argum);
    va_end(argum);
    fprintf(stream,"\n");
}

/* Príklad vyhľadávacej funkcie */
int xml_tagnamesearch(XMLTag *elem) 
{
    if (biseqcstr(elem->tagname, "p")) 
        return 1;   
    else
        return 0;
}

/* Poskytuje možnosť zapojiť vlastnú funkciu do prístupu ku
 * všetkým elementom - return 0 -> zobraziť text
 *                            1 -> nezobraziť text, ak je ptr NULL
 *                            zobrazí sa všetko */
void xml_treego(XMLTag *root, FILE *stream, int (*search)(XMLTag *elem))
{
    static int treelvl = 0; 
    size_t i;
    int display = 1;
    XMLTag **tagiter;  
    XMLAtribut *atriter;

    if (root == NULL) 
        return;
    
    if (search != NULL) 
        display = search(root);

    if (display) {
        xml_tabprint(treelvl, stream, "Element: %s", bdata(root->tagname));
        if (root->atribut != NULL) {        
            for (i = 0; i < vector_count(root->atribut); i++) {
                atriter = vector_at(root->atribut, i);
                xml_tabprint(treelvl, stream, "Key: %s; Value: %s", 
                        bdata(atriter->key), bdata(atriter->value));
            }
        }

        if (root->text != NULL) {
            xml_tabprint(treelvl, stream, "Text: %s", bdata(root->text));
        }
    }

    if (root->downtags != NULL) {
        for (i = 0; i < vector_count(root->downtags); i++) {
            tagiter = vector_at(root->downtags, i);
            treelvl++;
            xml_treego(*tagiter, stream, search);
            treelvl--;
        }
    }
} 

void xml_freetree(XMLTag *root)
{
    if (root != NULL)
        delete_tag(&root);      
}

static void print_error(const char *fmt, ...)
{
    va_list args;
    int i, begpos, endpos;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    begpos = bstrrchrp(g_xmltext,'<', g_filepos);
    endpos = bstrchrp(g_xmltext,'>', g_filepos);
    fprintf(stderr, "\t");
    for (i = begpos; i <= endpos; i++) {
        putc(bchar(g_xmltext, i), stderr);
    }
    fputs("\n\t", stderr);
    for (i = begpos; i < g_filepos; i++) 
        putc(' ', stderr);    
    fputs("^~~~~\n", stderr);
}

/* Vráti ukazateľ na hlavu syntaktického stromu*/
static XMLTag *xml_buildtree(void) 
{
    XMLTag *tag, *down;

    if (g_xmltext == NULL || g_xmltext->data == NULL || g_xmltext->slen <= 0 
        || g_xmltext->slen <= g_filepos || g_filepos < 0) {
        return NULL; 
    }

    /* Získaj tag */
    tag = xml_taginit();
    tag->tagname = xml_gettag();
    if (!tag->tagname) { 
        print_error("Chyba: Nedostatok pamate/ Neocakavany EOF\n");
        exit(1);
    }
    
    if (istag_closing(tag->tagname)) {
        return tag;
    }

    tag->atribut = xml_atributelist();
    
    g_filepos = bstrchrp(g_xmltext, '>', g_filepos);
    if (g_filepos == BSTR_ERR || bchar(g_xmltext, g_filepos + 1) == '\0') {
        print_error("Chyba: Neocakavany koniec suboru\n");
        exit(1);
    }

    if (bchar(g_xmltext, g_filepos - 1) == '/')   
        return tag;      /*  Self contained tag ==> close */

    /* Tag obsahuje text, prečítaj ho po ďalší tag*/ 
    ++g_filepos;
    tag->text = xml_tagtext();

      /* Rekurzia dole po strome */
     tag->downtags = vector_create(0, sizeof(XMLTag *), delete_tag);
     for(;;) {
        down = xml_buildtree();
        if (down == NULL)
            break;
        /* putchar('\n');xml_treetravel(down);,putchar('\n');
         * -- Zapnúť ak chceme vidieť vytváranie stromu*/ 
        if (istag_closing(down->tagname)) {
            bassignmidstr(down->tagname, down->tagname, 1, 
                                    blength(down->tagname));

            if (bstrcmp(down->tagname, tag->tagname) != 0) {
                print_error("Chyba - tag mismatch: '<%s>' je zatvoreny "
                            "ale posledny otvoreny je '<%s>'\n", 
                            bdata(down->tagname), bdata(tag->tagname));
                exit(1);
            } else {
                delete_tag(&down);
                return tag;
            }
        }
        vector_push_back(tag->downtags, &down); 
        /* Aj po end tagu pokračuj v parse*/
    }
    return tag;
}

static Vector *xml_atributelist(void)
{
    XMLAtribut kv;
    char begch;
    Vector *v = vector_create(0, sizeof(XMLAtribut), delete_xmlatrib); 
    
    while (bchar(g_xmltext, g_filepos) != '>' 
            && bchar(g_xmltext, g_filepos) != '/'
            && bchar(g_xmltext, g_filepos) != '\0') {
        
        kv.key = xml_getlextoken('='); 
        if (!kv.key) 
            return NULL; 
        
        if (bchar(g_xmltext, g_filepos) != '=') {
            print_error("Chyba: Ku klucu atributu neexistuje hodnota\n");
            exit(1);
        }

        /* Parse - (key="value") / (key='value')*/ 
        ++g_filepos; 
        if ((begch = bchar(g_xmltext, g_filepos)) != '"' && begch != '\'') {
             print_error("Chyba: Chybajuce otvaracie uvodzovky/apostrofy\n");
             exit(1);
        }
        ++g_filepos; /* preskočenie na prvý znak za úvodzovkami */
        
        kv.value = xml_getlextoken(begch);
        if (bchar(g_xmltext, g_filepos) != begch) {
            print_error("Chyba: Chybajuce uzatvarajuce uvodzovky/apostrofy\n");
            exit(1);
        }
        ++g_filepos; /* preskočenie za úvodzovky */
        while (isspace(bchar(g_xmltext, g_filepos))) 
            ++g_filepos;    /* preskočenie bielych znakov*/

        vector_push_back(v, &kv); 
    }

    if (vector_count(v) == 0) {
        vector_release(v);
        return NULL;
    }
    
    return v;
}

static XMLTag *xml_taginit(void) 
{
    XMLTag *tag = malloc(sizeof(XMLTag));
    if (tag == NULL)
        return NULL;

    tag->tagname = NULL; 
    tag->atribut = NULL;
    tag->text = NULL;
    tag->downtags = NULL;

    return tag;
}

static void delete_tag(void *item)
{
    bdestroy((*(XMLTag **)item)->tagname);
    if ((*(XMLTag **)item)->atribut != NULL) {
        vector_release((*(XMLTag **)item)->atribut);
    }
    bdestroy((*(XMLTag **)item)->text);
    if ((*(XMLTag **)item)->downtags != NULL) {
        vector_release((*(XMLTag **)item)->downtags);
    } 
}

static void delete_xmlatrib(void *data)
{
    bdestroy((*(XMLAtribut *)data).key);
    bdestroy((*(XMLAtribut *)data).value); 
}

static bstring xml_getlextoken(char terminator)
{
    char z;
    bstring word = bfromcstr("");
    
    /* Preskočíme všetky medzery medzi < a názvom tagu */
    for ( ; isspace(z = bchar(g_xmltext, g_filepos)); g_filepos++) {
        if (z == '\0')  
            return NULL;
    }

    /* číta po terminátor alebo koniec tagu */
    while ((z = bchar(g_xmltext, g_filepos)) != terminator && z != '>') {
        if (z == '\0') {
            bdestroy(word);
            return NULL;
        } 
        if ((bchar(g_xmltext, g_filepos)) == '/' 
            && bchar(g_xmltext, g_filepos + 1)== '>') {
            break;
        }
        bconchar(word, z);
        ++g_filepos;
    }

    /* nastav sa ďalší nebiely znak */
    while (isspace(z = bchar(g_xmltext, g_filepos)) && z != '\0')
        ++g_filepos;

    if (!blength(word) || z == '\0') {
        bdestroy(word);
        return NULL;
    }
    return word;
}

static bstring xml_gettag(void)
{
    char ch;

    if (g_xmltext == NULL || g_xmltext->data == NULL 
        || g_xmltext->slen <= g_filepos || g_filepos < 0)
		return NULL;

    g_filepos = bstrchrp(g_xmltext, '<', g_filepos);
    if (g_filepos == BSTR_ERR || bchar(g_xmltext, g_filepos + 1) == '\0') 
        return NULL;
    ++g_filepos;

    /* Preskoč deklaratívne tagy !-- , ?xml */
    if ((ch = bchar(g_xmltext, g_filepos)) == '!' || ch == '?') {
        return xml_gettag();
    }

    return xml_getlextoken(' ');
}

static bstring xml_tagtext(void)
{
    char z;
    bstring txt = bfromcstr("");
    if (!txt) 
        return NULL;

    while ((z = bchar(g_xmltext, g_filepos)) != '<' && z != '\0') {
        bconchar(txt, z);
        ++g_filepos;
    }

    if (!blength(txt)) {
        bdestroy(txt);
        return NULL;
    }

    return txt;
}
