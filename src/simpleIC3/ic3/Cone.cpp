/****************************************************************************************[Solver.h]
 The MIT License (MIT)

 Copyright (c) 2016, Sam Bayless

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

/*
 * Provides Cone of Influence detection
 */

#include <iostream>
#include "Cone.h"




using namespace SimpIC3;




void ConeOfInfluence::markCone( int l){
	//traverse the aig from each output, marking nodes as either relevant or irrelevant.
	int v = aiger_lit2var(l);

	if(!seen[v]){
		seen[v]=true;
		if(aiger_and* a = aiger_is_and(mgr,aiger_strip(l))){
			int l0= a->rhs0;
			markCone(l0);

			int l1= a->rhs1;
			markCone(l1);
		}else if (aiger_is_input(mgr,aiger_strip(l))){
			//done
		}else if(aiger_symbol * s = aiger_is_latch(mgr,aiger_strip(l))){
			//done
		}

	}
}



void ConeOfInfluence::collectCOI( int l,Bitset & store){

	seen.clear();
	seen.growTo(mgr->maxvar+1);
	store.clear();
	store.growTo(mgr->num_latches);

	markCone(l);

	for(int i = 0;i<mgr->num_latches;i++){
		aiger_symbol& s = mgr->latches[i];
		int l= s.lit;
		  int res = aiger_lit2var (l);
		  if(seen[res]){
			  store.set(i);
		  }
	}


}

