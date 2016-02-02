/*
 * Provides Cone of Influence detection
 * Cone.cpp
 *
 *  Created on: 2013-04-30
 *      Author: sam
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

