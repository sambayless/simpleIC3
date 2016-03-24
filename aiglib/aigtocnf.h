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
#ifndef PARSE_AIG
#define PARSE_AIG


extern "C" {
#include "aiger/aiger.h"
}
#include <minisat/core/Solver.h>
#include <minisat/simp/SimpSolver.h>
#include <minisat/core/SolverTypes.h>
#include <minisat/mtl/Vec.h>

using namespace Minisat;
namespace aiglib{
static int
lit2int (aiger * mgr, unsigned a, int startVar)
{
  int sign = aiger_sign (a) ? -1 : 1;
  int res = aiger_lit2var (a)+1;

  //if (res){
    res = (res+startVar)* sign;
/*
  }else
    res = sign;	//assume true and false are 0 and 1
*/
 // assert(res!=0);
  return res;
}

template<class Solver>
 Minisat::Lit getLit(Solver & S, int dimacs){
	if (dimacs == 0) return lit_Undef;
	int v = abs(dimacs)-1;

	while (v >= S.nVars()) S.newVar();
    return( (dimacs > 0) ? mkLit(v) : ~mkLit(v) );
}
template<class Solver>
 Minisat::Lit getLit(Solver & S,const int startVar, aiger * mgr, unsigned lit){

/*	if(aiger_symbol * s = aiger_is_latch(mgr,aiger_strip(lit))){
		 int regnum = s - mgr->latches;
		  int res = aiger_lit2var (lit);

		  assert(aiger_lit2var( s->lit)==res);

		//this is a previous state - use the variable already in the solver.
		  assert(regnum<in_latches.size());
		  //Var v = in_latches[regnum];
		  Minisat::Lit l = in_latches[regnum];
		  return mkLit(var(l),((bool) aiger_sign (lit)) ^ sign(l) );

	}else{*/
	bool sign = aiger_sign(lit);
	int dimacs = lit2int(mgr,lit,startVar);

	return getLit(S,dimacs);
	//}


}
using namespace Minisat;
template<class Solver>
 Lit aigtocnf(Solver & S, aiger * mgr,vec<Minisat::Lit> & inputs, vec<Minisat::Lit> & in_latches, vec<lbool> & latch_reset, vec<Minisat::Lit> & out_latches, vec<Minisat::Lit> & outputs,vec<Lit> & badStates){

		vec<Minisat::Lit> c;


		int startVar = S.nVars();
		//first lit is 'true'
		Minisat::Lit True = getLit(S,startVar,mgr,aiger_true);
		S.addClause(True);
		Minisat::Lit False = getLit(S,startVar,mgr,aiger_false);
		assert(False==~True);

		//construct the (non-latch) inputs to the circuit (these are required by ternary simulation). Since the ground literal and latch inputs are already allocated
		//input i will be assigned variable i+1+num_latches
	 for (int i = 0; i < mgr->num_inputs; i++)
	{
		 Minisat::Lit in = getLit(S,startVar, mgr,mgr->inputs[i].lit);
		 inputs.push(in);
	}

	  for (int i = 0; i < mgr->num_ands; i++)
	    {
	      aiger_and *a = mgr->ands + i;
	      c.push( getLit(S,startVar, mgr, aiger_not (a->lhs)));
	      c.push(getLit(S,startVar, mgr, a->rhs0));
	      S.addClause(c);
	      c.clear();

	      c.push( getLit(S,startVar, mgr, aiger_not (a->lhs)));
	      c.push(getLit(S,startVar, mgr, a->rhs1));
	      S.addClause(c);
	      c.clear();


	      c.push(getLit(S,startVar, mgr, a->lhs));
	      c.push( getLit(S,startVar, mgr, aiger_not (a->rhs0)));
	      c.push( getLit(S,startVar, mgr, aiger_not (a->rhs1)));
	        S.addClause(c);
	        c.clear();

	    }

	  //check if any latches have input latches as their 'next' function; if so, construct a variable for their function
	  for(int i = 0;i<mgr->num_latches;i++){
		  getLit(S,startVar,mgr,mgr->latches[i].next);
		  //****This loop IS critical, even though getLit's return value is ignored, because it forces the construction of a variable for the next function in the (rare) case that
		  //that variable hasn't yet been defined. This can happen if the next value of a latch is an input latch, and that input latch is not referenced anywhere else in the circuit.
		  //Constructing this variable is necessary before the final output latch variables are added below
	  }

	  for(int i = 0;i<mgr->num_latches;i++){

			//int dimacs = lit2int(mgr, mgr->latches[i].next,startVar);
				Minisat::Lit l = getLit(S,startVar,mgr,mgr->latches[i].next);
				Minisat::Lit p =  getLit(S,startVar,mgr,mgr->latches[i].lit);

				in_latches.push(p);
				out_latches.push(l);

				if(mgr->latches[i].reset==aiger_true){
					latch_reset.push(l_True);
					//S.addClause(p);
				}else if (mgr->latches[i].reset==aiger_false){
					latch_reset.push(l_False);
					//S.addClause(~p);
				}else{
					//else, p is unconstrained
					latch_reset.push(l_Undef);
				}


	  }
	  c.clear();
	  for(int i = 0;i<mgr->num_constraints;i++){
			Minisat::Lit p =  getLit(S,startVar,mgr,mgr->constraints[i].lit);
			S.addClause(p);
	  }

	  for(int i = 0;i<mgr->num_outputs;i++){
		Minisat::Lit p =  getLit(S,startVar,mgr,mgr->outputs[i].lit);
		outputs.push(p);
	  }

	  for(int i = 0;i<mgr->num_bad;i++){
			Minisat::Lit p =  getLit(S,startVar,mgr,mgr->bad[i].lit);
			badStates.push(p);
	  }

	  return True;
	}
};

#endif
