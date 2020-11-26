typedef struct sFile_data tFile_data;

typedef struct sList_Element{
    struct sList_Element * next_element;
    void                 * item;
}tList_Element;

typedef struct sList{
    tList_Element * first_element;
    int size;
}tList;

tList * create_list(void);
tList_Element * get_element(tList * list, int index);
void append_element(tList * list, void * new_item);
void delete_list(tList * list);