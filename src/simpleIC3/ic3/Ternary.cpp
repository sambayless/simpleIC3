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


#include "Ternary.h"
extern "C" {
#include "aiger/aiger.h"
}

using namespace Minisat;
//uncomment to enable expensive debugging checks
//#define DEBUG_IC3
using namespace SimpIC3;
	Ternary::Ternary(aiger * _aig,vec<int> * _priority):aig(_aig),priority(_priority){

		 //inputs were assigned variables i+1+num_latches
		 min_primary_input=1+aig->num_latches;
		 max_primary_input=min_primary_input+aig->num_inputs;

		fanouts.growTo(aig->maxvar+1);
		for(int i=0;i<aig->num_ands;i++){
			aiger_and & a = aig->ands[i];
			int vfor = aiger_lit2var(a.lhs);
			fanouts[aiger_lit2var(a.rhs0)].push(vfor);
			fanouts[aiger_lit2var(a.rhs1)].push(vfor);
		}

		assigns.growTo(aig->maxvar+1);
		preserve.growTo(aig->maxvar+1);


		empty.growTo(aig->maxvar+1);


	 }

	 bool  Ternary::propagate( unsigned lit){
		if(aiger_and * a = aiger_is_and(aig,lit)){
			lbool cur = assigns[aiger_lit2var(lit)];
			int input0 = a->rhs0;
			int var0 = aiger_lit2var(input0);
			lbool assign0 = assigns[var0]^aiger_sign(input0);

			int input1 = a->rhs1;
			int var1 = aiger_lit2var(input1);
			lbool assign1 = assigns[var1]^aiger_sign(input1);

			lbool assign= assign0&& assign1;

			if(toInt(assign)>toInt(l_Undef)){
						assign=l_Undef;
						}

			assigns[aiger_lit2var(lit)]=assign;
			assert((assigns[aiger_lit2var(lit)]==l_False) || (assigns[aiger_lit2var(lit)]==l_True) || (assigns[aiger_lit2var(lit)]==l_Undef));
			return assigns[aiger_lit2var(lit)]!=cur;
		}
		return false;
	}





	bool  Ternary::simulate(int input_var,lbool assign, vec<bool> & preserve ){

		seen.clear();
		seen.growTo(aig->maxvar+1);
		to_update.clear();
		to_update.push(input_var);

		if(preserve[input_var])
			return false;


		seen[input_var]=true;
		lbool old_assign = assigns[input_var];
		if(old_assign==assign){
			return true;
		}

#ifndef DEBUG_IC3
		vec<lbool> start_state;
		assigns.copyTo(start_state);
#endif

		assigns[input_var]=assign;

		for(int i = 0;i<to_update.size();i++){
			int v = to_update[i];
			seen[v]=false;
			vec<int> & fanout = fanouts[v];
			for(int j = 0;j<fanout.size();j++){
				int o = fanout[j];

				if(!seen[o]){
					if(propagate(aiger_var2lit(o))){
						seen[o]=true;
						if(preserve[o]){
							simulate(input_var, old_assign,empty);
#ifndef DEBUG_IC3
							for(int i = 0;i<assigns.size();i++){
								assert(assigns[i]==start_state[i]);
							}
#endif
							return false;
						}
						to_update.push(o);
					}
				}
			}
		}
		return true;
	}




	void Ternary::reduce(vec<lbool>& latch_assignment,vec<lbool>& primary_input_assignments, vec<int> & to_preserve){


		for(int i = 0;i<assigns.size();i++){
			assigns[i]=l_Undef;
			preserve[i]=false;
		}

		assigns[0]=l_False; //ground literal is constant

		for(int i = 0;i<to_preserve.size();i++){
			int aiger_lit =  to_preserve[i];
			preserve[aiger_lit2var(aiger_lit)]=true;
		}

		for(int i = 0;i<latch_assignment.size();i++){
			int latch_num = i;
			lbool assign = latch_assignment[i];
			int lit = aig->latches[latch_num].lit;
			int v = aiger_lit2var(lit);
			assigns[v]=assign;
		}

		for(int i = 0;i<primary_input_assignments.size();i++){

			//assert(in_latches[latch_num]==var(assignment[i]));
			int lit = aig->inputs[i].lit;

			int v = aiger_lit2var(lit);
			assert(assigns[v]==l_Undef);
			assigns[v]=primary_input_assignments[i];
		}

		//propagate those assignments through the circuit
		for(int i = 0;i<=aig->maxvar;i++){
			int l = aiger_var2lit(i);
			propagate(l);
		}

#ifndef DEBUG_IC3
		static vec<lbool> start_assign;
		assigns.copyTo(start_assign);
#endif

		//from ABC's PDR implementation

		// try removing high priority flops
		if(priority){
			for(int i = 0;i<latch_assignment.size();i++){

				int latch_num = i;
				if (latch_assignment[i]!=l_Undef){
					int p = (*priority)[latch_num];
					if(p>0){
						int lit = aig->latches[latch_num].lit;
						simulate(aiger_lit2var(lit),l_Undef,preserve);
					}
				}
			}
		}

		// try removing low priority flops
		for(int i = 0;i<latch_assignment.size();i++){
			if (latch_assignment[i]!=l_Undef){
				int latch_num = i;
				int p = priority? (*priority)[latch_num]:0;
				if(p<=0){
					int lit = aig->latches[latch_num].lit;
					simulate(aiger_lit2var(lit),l_Undef,preserve);
				}
			}
		}



		for(int i = 0;i<latch_assignment.size();i++){
			int latch_num = i;
			//assert(in_latches[latch_num]==var(l));
			int lit = aig->latches[latch_num].lit;
			lbool assign = assignment(lit);
			latch_assignment[i]=assign;
		}


#ifndef DEBUG_IC3

			for(int i = 0;i<preserve.size();i++){
				if(preserve[i]){
					assert(assigns[i]==start_assign[i]);
				}
			}
#endif


	}

