#pragma once

#include "geom.h"
#include "xarray.h"

#define NOT_IN_HEAP -47

//
//
// This file extracted from ~/anim/lab/mlab
//
//

class heap_node {
public:
    double import;
    Labelled *obj;

    heap_node() { obj=NULL; import=0.0; }
    heap_node(Labelled *t, double i=0.0) { obj=t; import=i; }
    heap_node(const heap_node& h) { import=h.import; obj=h.obj; }
};



class Heap : public xarray<heap_node> {

    //
    // The actual size of the heap.  xarray::length()
    // simply returns the amount of allocated space
    int size;

    void swap(int i, int j);

    int parent(int i) { return (i-1)/2; }
    int left(int i) { return 2*i+1; }
    int right(int i) { return 2*i+2; }

    void upheap(int i);
    void downheap(int i);

public:

    Heap() { size=0; }
    Heap(int s) : xarray<heap_node>(s) { size=0; }


    void insert(Labelled *, double);
    void update(Labelled *, double);

    heap_node *extract();
    heap_node *top() { return size<1 ? (heap_node *)NULL : &ref(0); }
    heap_node *kill(int i);
};

