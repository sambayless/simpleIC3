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
//Implements PDR-style ternary simulation for minimizing proof obligations
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
