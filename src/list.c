#include <stdio.h>
#include <stdlib.h>
#include "list.h"

tList * create_list(void){
    tList * list = malloc(sizeof(tList));
    list->first_element = NULL;
    list->size = 0;
}


tList_Element * get_element(tList * list, int index){
    if(index < 0 || index >= list->size){
        return NULL;
    }
    int i;
    tList_Element * element_at_index = NULL;
    tList_Element * tmp = list->first_element;
    for(i = 0; i <= index; i++){
        if(i == index){
            element_at_index = tmp;
            break;
        }
        tmp = tmp->next_element;
    }

    return element_at_index;
}

void append_element(tList * list, tFile_data * new_item){
    tList_Element * element = malloc(sizeof(tList_Element));
    element->item = new_item;
    element->next_element = NULL;
    if(list->size == 0){
        list->first_element = element;
    } else {
        tList_Element * tmp_last = get_element(list, list->size - 1);
        tmp_last->next_element = element;
    }
    list->size++;
}

void delete_list(tList * list){
    int i;
    tList_Element * tmp_next_element;
    for(i = 0; i < list->size - 1; i++){
        tList_Element * current_element;
        if(i == 0){
            current_element = list->first_element;
        } else {
            current_element = tmp_next_element;
        }
        tmp_next_element = current_element->next_element;
        free(current_element->item);
        free(current_element);
    }
    free(tmp_next_element->item);
    free(tmp_next_element);
    free(list);
}