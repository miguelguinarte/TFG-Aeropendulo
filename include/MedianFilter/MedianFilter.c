#include <stdio.h>
#include "MedianFilter.h"


#define STOPPER                 (-1000)
#define MEDIAN_FILTER_WINDOW    (9)   

static struct node buffer[MEDIAN_FILTER_WINDOW] = { 0 };
static struct node *iterator = buffer;
static struct node smaller = { NULL, STOPPER };
static struct node bigger = { &smaller, 0 };

float MedianFilter(float newValue){
    struct node *successor;
    struct node *accessor;
    struct node *accessorPrev;
    struct node *median;
    int i;
    if (newValue == STOPPER)
        newValue++;
    if ((++iterator - buffer) >= MEDIAN_FILTER_WINDOW)
        iterator = buffer;
    iterator->value = newValue;
    successor = iterator->next;
    median = &bigger;
    accessorPrev = NULL;
    accessor = &bigger;
    if (accessor->next == iterator)
        accessor->next = successor;
    accessorPrev = accessor;
    accessor = accessor->next;
    for (i = 0; i < MEDIAN_FILTER_WINDOW; ++i)
    {
        if (accessor->next == iterator)
            accessor->next = successor;
        if (accessor->value < newValue)
        {
            iterator->next = accessorPrev->next;
            accessorPrev->next = iterator;
            newValue = STOPPER;
        };
        median = median->next;
        if (accessor == &smaller)
            break;
        accessorPrev = accessor;
        accessor = accessor->next;
        if (accessor->next == iterator)
            accessor->next = successor;
        if (accessor->value < newValue)
        {
            iterator->next = accessorPrev->next;
            accessorPrev->next = iterator;
            newValue = STOPPER;
        }
        if (accessor == &smaller)
            break;
        accessorPrev = accessor;
        accessor = accessor->next;
    }
    return median->value;
}

float Median3(float a,float b,float c){
    if ((a <= b) && (a <= c))
		return (b <= c) ? b : c;
	if ((b <= a) && (b <= c))
		return (a <= c) ? a : c;
	return (a <= b) ? a : b;
}

float AddValue3(float value){
    if ((++iterator - buffer) >= 3)
		iterator = buffer;
	iterator->value = value;

	return Median3(buffer->value, (buffer + 1)->value, (buffer + 2)->value);
}

void ArrayDisplacement(float *array,int dim){
    for(int i=dim;i>0;i--){
        array[dim] = array[dim - 1];
    }
}