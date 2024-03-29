#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "list.h"

int main(int argc, char * argv[]){
    tList * list = create_list();
    int i;
    time_printf(("Starting to insert\n"));
    for(i = 0; i < 5; i++){
        float * tmp = malloc(sizeof(float));
        *tmp = (float) 1.0 * i;
        time_printf(("List-size: %i\n", list->size));
        append_element(list, tmp);
    }

    time_printf(("\nStarting to print\nFirst_Element: %i\n", *((float *)list->first_element->item)));
    for(i = 0; i < 5; i++){
        tList_Element * tmp = get_element(list, i);
        time_printf(("Element %i has value %i\n", i, *((float *)tmp->item)));
    }

    time_printf(("\nDeleting list\n"));
    delete_list(list);
    time_printf(("\nAll good\n"));
    return 0;
}