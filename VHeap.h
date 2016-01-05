/*
 * VectorQueue.h
 *
 *  Created on: 2013-03-18
 *      Author: sam
 */

#ifndef VECTORHEAP_H_
#define VECTORHEAP_H_


#include <minisat/mtl/Vec.h>
#include <minisat/mtl/Queue.h>
#include <minisat/mtl/Heap.h>

using namespace Minisat;
namespace SimpIC3{

template<class T>
class VHeap {
    vec<vec<T> >  elements;
    int sz;

    struct IntCmp{
    	 bool operator()(int a, int b)const{
    		return a<b;
    	}
    	 IntCmp(){};
    };
    Heap<int,IntCmp> Q;


public:

    VHeap():sz(0), Q(IntCmp()){

    }

    void insert(int key,const  T & element){
    	sz++;
    	Q.update(key);
    	elements.growTo(key+1);
    	elements[key].push(element);
    }

    bool empty(){
    	return sz==0;
    }

    bool contains(int key){
    	return key< elements.size() && elements[key].size()>0;
    }

    T & getLast(int key){
    	assert(contains(key));
    	return elements[key].last();
    }

    int size(){
    	return sz;
    }

    T & peekMin(){
    	assert(!empty());
    	assert(Q.size()>0);
    	int level = Q.peekMin();
    	return elements[level].last();
    }

    T & removeMin(){

    	assert(!empty());
		assert(Q.size()>0);
		int level = Q[0];
		assert(level<elements.size());
		T & element = elements[level].last();
		elements[level].pop();
		if(elements[level].size()==0){
			Q.removeMin();
		}
		sz--;
		return element;
    }

    void clear(bool dealloc = false){
    	Q.clear(dealloc);

    	//this might not be necessary if dealloc is true?
    	for(int i = 0;i<elements.size();i++){
    		elements[i].clear(dealloc);
    	}
    	if(dealloc){
    		elements.clear(true);
    	}
    }




};
};



#endif /* VECTORHEAP_H_ */
