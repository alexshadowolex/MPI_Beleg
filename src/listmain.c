#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "list.h"

int main(int argc, char * argv[]){
    tList * list = create_list();
    int i;
    printf("Starting to insert\n");
    for(i = 0; i < 5; i++){
        int * tmp = malloc(sizeof(int));
        *tmp = i;
        printf("List-size: %i\n", list->size);
        append_element(list, tmp);
    }

    printf("\nStarting to print\nFirst_Element: %i\n", *list->first_element->item);
    for(i = 0; i < 5; i++){
        tList_Element * tmp = get_element(list, i);
        printf("Element %i has value %i\n", i, *tmp->item);
    }

    printf("\nDeleting list\n");
    delete_list(list);
    printf("\nAll good\n");
    return 0;
}