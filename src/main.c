#include <stdio.h>
#include "bstrlib.h"
#include "xmlparser.h"

int main(int argc, char *argv[]) 
{
    FILE *xmlfile = NULL;
    bstring input = bfromcstr("");
    XMLTag *treehead;

    if (argc != 2) {
        printf("Zadajte XML/XHTML na parsing (syntakticku analyzu): ");
        input = bgetline(stdin);
    } else {
        bassigncstr(input, argv[1]);
    }
    
    xmlfile = fopen(bdata(input), "r"); 
    if (!xmlfile) {
        perror("Chyba pri otvarani suboru");
        bdestroy(input);
        return 1;
    }
    /* xml parse */
    treehead = xml_parse(xmlfile);   
    xml_treego(treehead, stdout, NULL);
    //xml_treego(treehead, stdout, xml_tagnamesearch);
    
    /* clean up */
    xml_freetree(treehead);
    bdestroy(input);
    fclose(xmlfile);
} 
