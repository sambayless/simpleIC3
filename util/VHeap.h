/****************************************************************************************[Solver.h]
 The MIT License (MIT)

 Copyright (c) 2014, Sam Bayless

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

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
