/*
 * Ternary.h
 *
 *  Created on: 2013-05-01
 *      Author: sam
 */

#ifndef TERNARY_H_
#define TERNARY_H_

extern "C" {
#include "aiger/aiger.h"
}
#include <minisat/mtl/Vec.h>
#include <minisat/core/SolverTypes.h>


using namespace Minisat;
namespace SimpIC3{
class Ternary{

	 vec<lbool> assigns;
	 vec<vec<int> > fanouts;


	 aiger * aig;
	 vec<int> * priority;
	  vec<int> to_update;
	 vec<bool> seen;
	 vec<bool> empty;
	 vec<bool> preserve;

public:

	 int min_primary_input;
	 int max_primary_input;

	 Ternary(aiger * _aig,vec<int> * _priority=NULL);
private:

	bool propagate( unsigned lit);

	bool simulate(int input_var,lbool assign, vec<bool> & preserve);

	lbool assignment(int l){
		return assigns[aiger_lit2var(l)] ;
	}

public:
	//minisat interface

	/**
	 * Primary Input assignments is the complete set of assignments to the (non-latch) inputs to the circuit
	 */
	void reduce(vec<lbool>& latch_assignment,vec<lbool>& primary_input_assignments, vec<int>& to_preserve);

};
};


#endif /* TERNARY_H_ */
