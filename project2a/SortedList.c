#include "SortedList.h"
#include <stdio.h>
#include <string.h>
#include <sched.h>

void SortedList_insert(SortedList_t *list,SortedListElement_t *element){
  SortedListElement_t *traveler=list->next;
  if(list==NULL || element == NULL){return;}
  while(traveler!= list && strcmp(traveler->key,element->key)>0){
    traveler=traveler->next;
  }
  if(opt_yield & INSERT_YIELD)
    {sched_yield();}
  element->prev=traveler->prev; //might lose this connection
  traveler->prev->next=element;
  traveler->prev=element;
  element->next=traveler;
  
}

int SortedList_delete( SortedListElement_t *element){
  if(element->key==NULL || element==NULL || element->prev->next !=element || element->next->prev != element)
  {return 1;}
  if(opt_yield & DELETE_YIELD)
    {sched_yield();}
  element->prev->next=element->next;
  element->next->prev=element->prev;
  return 0;
}

SortedListElement_t* SortedList_lookup(SortedList_t *list, const char * key){
  if(list ==NULL || key==NULL){return NULL;}
  SortedListElement_t *traveler=list->next;
  while(traveler!=list){
    if(strcmp(traveler->key, key)==0)
      {return traveler;}
    if(opt_yield & LOOKUP_YIELD)
      {sched_yield();}
    traveler=traveler->next;
  }
  return NULL;
}

int SortedList_length(SortedList_t *list){
  if(list->key !=NULL || list==NULL){return -1;} //the two criterias
  int sum=0;
  SortedListElement_t * traveler=list->next;
  while(traveler!=list){
    if(traveler->prev->next != traveler || traveler->next->prev != traveler)
      {return -1;}
    sum++;
    if(opt_yield & LOOKUP_YIELD)
      {sched_yield();}
    traveler=traveler->next;
  }
  return sum;
}
