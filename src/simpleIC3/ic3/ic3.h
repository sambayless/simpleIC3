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
#ifndef IC3_H_
#define IC3_H_

#include <minisat/core/Solver.h>
#include <minisat/simp/SimpSolver.h>
#include <minisat/mtl/Vec.h>
#include <minisat/mtl/Sort.h>
#include <minisat/core/Dimacs.h>

#include "simpleIC3/util/VHeap.h"
#include "Cone.h"
#include "Ternary.h"
#include "simpleIC3/util/Bitset.h"
#include <stdexcept>
#include "ic3config.h"
#include "Cone.h"
#include "ic3Types.h"
using namespace Minisat;
namespace SimpIC3{
//uncomment to enable expensive debugging checks
//#define DEBUG_IC3
class IC3{

	SimpSolver & S;
	aiger * mgr;
	Ternary * ternary=nullptr;
	Bitset property_coi;

	ConeOfInfluence * coi=nullptr;
	vec<Bitset> latch_coi;
	Bitset cur_coi;
	Bitset naive_tern;



	vec<int> ternary_preserve;
	vec<lbool>ternary_inputs;
	vec<lbool> ternary_assign;

	vec<Var>&  in_latches;
	vec<Var> & out_latches;
	vec<lbool> & resets;
	vec<Var> & primary_inputs;

	Lit lit_True;
	vec<vec<Lit> > clauses;//clauses of proof
	vec<Lit> activations;//activation lits for each clause
	vec<int> clause_frame;//frame that each clause belongs to
	vec<uint32_t> abstractions;//abstractions for each clause. -1 if clause has already been subsumed.
	vec<bool> is_subsumed;
	vec<int> lit_to_clause;//gets the clause corresponding to an activation variable
	vec<vec<Lit>> frames; //activation lits for each frame

	vec<int> in_latch_to_index;
	vec<int> out_latch_to_index;

	Var start_of_activation_vars=var_Undef;
	Var start_of_in_latches=var_Undef;
	Var start_of_out_latches=var_Undef;

	vec<int> priority;

	int num_gen;
	int num_fail_blocked;

	vec<int> act_vars;
	//temporary vectors
	vec<char> seen_subsume;
	vec<Lit> pre;
	vec<Lit> pre_inputs;

	bool isActivationLit(Lit activationLit){
		Var v = var(activationLit);
		if(v<start_of_activation_vars)
			return false;

		int index = v-start_of_activation_vars;

		if(index>=lit_to_clause.size()){
			return false;
		}
		return lit_to_clause[index]>=0;
	}
	int getClauseFrame(int clauseN){
		return clause_frame[clauseN];
	}
	int getClauseFromActivation(Lit activationLit){
		assert(isActivationLit(activationLit));
		Var v = var(activationLit);
		assert(v>=start_of_activation_vars);
		int index = v-start_of_activation_vars;
		assert(index<lit_to_clause.size());
		return lit_to_clause[index];
	}

	bool satisfiesReset(Lit in_latch){
		int latch = inLatchN(var(in_latch));
		lbool r = resets[latch];
		if(r==l_Undef){
			return false;
		}
		return((r==l_True) == (!sign(in_latch)));
	}

	int outLatchN(Var outlatch){
		assert(outlatch>=start_of_out_latches);
		int index = outlatch-start_of_out_latches;
		assert(index<out_latch_to_index.size());
		int i =  out_latch_to_index[index];
		return i;
	}
	int inLatchN(Var inlatch){
		assert(inlatch>=start_of_in_latches);
		int index = inlatch-start_of_in_latches;
		assert(index<in_latch_to_index.size());
		int i =  in_latch_to_index[index];
		return i;
	}
	Var outToIn(Var outlatch){
		assert(isOutLatch(outlatch));
		return in_latches[outLatchN(outlatch)];
	}

	Var inToOut(Var inlatch){
		assert(isInLatch(inlatch));
		return out_latches[inLatchN(inlatch)];
	}

	Lit outToIn(Lit outlatch){
		Var v= outToIn(var(outlatch));
		return mkLit(v,sign(outlatch));
	}

	Lit inToOut(Lit inlatch){
		Var v= inToOut(var(inlatch));
		return mkLit(v,sign(inlatch));
	}

	void outToIn(vec<Lit> & lits){
		assert(hasOutLatches(lits));
		for(int i = 0;i<lits.size();i++){
			lits[i]=outToIn(lits[i]);
		}
	}

	void inToOut(vec<Lit> & lits){
		assert(hasInLatches(lits));
		for(int i = 0;i<lits.size();i++){
			lits[i]=inToOut(lits[i]);
		}
	}

	bool isInLatch(Var v){
		if(v<start_of_in_latches)
			return false;
		int index =v- start_of_in_latches;
		if(index>=in_latch_to_index.size())
			return false;
		return in_latch_to_index[index]>-1;
	}

	bool isInLatch(Lit l){
		return isInLatch(var(l));
	}

	bool hasInLatches(vec<Lit> & lits){
		for(int i = 0;i<lits.size();i++){
			if(!isInLatch(lits[i]))
				return false;
		}
		return true;
	}

	bool isOutLatch(Var v){
		if(v<start_of_out_latches)
			return false;
		int index =v- start_of_out_latches;
		if(index>out_latch_to_index.size())
			return false;
		return out_latch_to_index[index]>-1;
	}

	bool isOutLatch(Lit l){
		return isOutLatch(var(l));

	}
	bool hasOutLatches(vec<Lit> & lits){
		for(int i = 0;i<lits.size();i++){
			if(!isOutLatch(lits[i]))
				return false;
		}
		return true;
	}


	void releaseClause(Lit l){
		int clauseN = getClauseFromActivation(l);
		assert(isActivationLit(l));
		Var v = var(l);
		assert(v>=start_of_activation_vars);
		int index = v-start_of_activation_vars;
		assert(index<lit_to_clause.size());
		lit_to_clause[index]=-1;

		clauses[clauseN].clear();

		abstractions[clauseN]=0;
		clause_frame[clauseN]=-1;
		is_subsumed[clauseN]=false;
		//TODO: we can re-use this clause index, which will save on memory a bit
		S.releaseVar(~l);
	}

	VHeap<TCube*> Q;//Q of bad states to process

	vec<Lit> tmp_assumps;
	bool solveAtFrame(vec<Lit> & extra_assumptions, int frame){
		assert(frame>=0);
		assert(frame<frames.size());
		tmp_assumps.clear();

		for(int f = frame;f<frames.size();f++){
			vec<Lit> & acts = frames[f];
			for(int i = 0;i<acts.size();i++)
				tmp_assumps.push(acts[i]);
		}

		for(int i = 0;i<extra_assumptions.size();i++)
			tmp_assumps.push(extra_assumptions[i]);
		bool r = S.solve(tmp_assumps);
		return r;
	}
	bool pre_image(vec<Lit> & p, vec<int> & aiger_lits, vec<Lit> & store, vec<Lit> & store_inputs, bool optimize=true){
			store.clear();
			store_inputs.clear();
			bool r = solveAtFrame(p,depth());
			if(r){
				for(int i =0;i<in_latches.size();i++){
					if(S.modelValue(in_latches[i])==l_True){
						store.push(mkLit(in_latches[i]));
					}else{
						store.push(~mkLit(in_latches[i]));
					}
				}
				for(int i =0;i<primary_inputs.size();i++){
					if(S.modelValue(primary_inputs[i])==l_True){
						store_inputs.push(mkLit(primary_inputs[i]));
					}else{
						store_inputs.push(~mkLit(primary_inputs[i]));
					}
				}
			}
			if(!optimize)
				return r;

			//apply coi, ternary simulation
			if(opt_coi){
				int i,j=0;
				for(i = 0;i< store.size();i++){
					Lit l = store[i];
					int latchN = inLatchN(var(l));
					if(property_coi[i]){
						store[j++]=store[i];
					}else{
						stats_coi_lits_removed++;
					}
				}
				store.shrink(i-j);
			}
			 if(ternary){
				 ternary_assign.clear();
				 ternary_assign.growTo(in_latches.size(),l_Undef);
				ternary_preserve.clear();
				for(int i = 0;i<aiger_lits.size();i++){
					ternary_preserve.push(aiger_lits[i]);
				}
				for(int i = 0;i<store.size();i++){
					Lit l = store[i];
					int latchN = inLatchN(var(l));
					if(sign(l)){
						ternary_assign[latchN]=l_False;
					}else{
						ternary_assign[latchN]=l_True;
					}
				}

				ternary_inputs.clear();
				for(int i = 0;i<primary_inputs.size();i++){
					Var v = primary_inputs[i];
					ternary_inputs.push(S.modelValue(v));
				}


				ternary->reduce(ternary_assign,ternary_inputs,ternary_preserve);
				int i,j=0;
				for(i = 0;i<store.size();i++){
					Lit l = store[i];
					int latch = inLatchN(var(l));
					lbool v= ternary_assign[latch];
					if(v!=l_Undef){
						store[j++]=l;
					}else{
						stats_ternary_lits_removed++;
					}
				}
				store.shrink(i-j);
			 }
			return r;
		}

	void moveInvariantToEnd(int from){

		for(int f = from;f<frames.size()-1;f++){
			vec<Lit> & frame = frames[f];
			for(int i = 0;i<frame.size();i++){
				Lit l = frame[i];
				int clauseN = getClauseFromActivation(l);
				if(is_subsumed[clauseN]){
					releaseClause(l);
					continue;
				}

				frames.last().push(l);
				assert(clause_frame[clauseN]==f);
				clause_frame[clauseN]=frames.size()-1;

			}
			frame.clear();
		}

	}



	void buildCOI(vec<int> & aiger_lit_property){
		cur_coi.clear();
		cur_coi.growTo(in_latches.size());

		//how does this need to be modified to handle aiger constraints?
		//do all latches occuring in aiger constraints need to be in every cone?
		if(!coi){
			if(opt_verb>1){
				printf("Creating Cone-of-influence for each latch: 0");
			}
			coi = new ConeOfInfluence(mgr);
			latch_coi.clear();
			latch_coi.growTo(in_latches.size());
			for(int i = 0;i<in_latches.size();i++){
				Bitset & b = latch_coi[i];
				coi->collectCOI(mgr->latches[i].next,b);
				if(opt_verb>1){
					printf("\rCreating Cone-of-influence for each latch: %d",i);
				}
			}
			if(opt_verb>1){
				printf("Creating Cone-of-influence for each latch: 0\n");
			}
		}

		property_coi.clear();
		property_coi.growTo(in_latches.size());

		Bitset tmp;
		for(int i = 0;i<aiger_lit_property.size();i++){
			int aiger_lit = aiger_lit_property[i];
			coi->collectCOI(aiger_lit, tmp );
			property_coi.Or(tmp);
		}


	}
	vec<Lit> pre_tmp;
	bool isPreimage(vec<Lit> & pre,vec<Lit> & post){
		pre_tmp.clear();
		pre.copyTo(pre_tmp);
		for(int i = 0;i<post.size();i++){
			pre_tmp.push(post[i]);
		}

		return S.solve(pre_tmp);

	}

	long stats_subsumed_clauses_removed=0;
	long stats_generalize_lits_removed =0;
	long stats_ternary_lits_removed=0;
	long stats_coi_lits_removed=0;

public:
	void printStats(){

		printf("Generalize lits removed: %ld\n",stats_generalize_lits_removed);
		printf("Clauses removed by subsumption: %ld\n",stats_subsumed_clauses_removed);

		if(opt_coi){
			printf("COI lits removed: %ld\n",stats_coi_lits_removed);
		}
		if(opt_coi){
			printf("Ternary lits removed: %ld\n",stats_ternary_lits_removed);
		}
		printf("%ld TCubes total allocated (%ld TCubes still allocated)\n",TCube::allocatedTCubes(),TCube::remainingTCubes());
	}
	IC3(SimpSolver & S,aiger * mgr, vec<Var> & in_latches, vec<Var> & out_latches, vec<lbool> & resets,vec<Var> & primary_inputs ):S(S),mgr(mgr),in_latches(in_latches),out_latches(out_latches),resets(resets),primary_inputs(primary_inputs){
		lit_True = mkLit(S.newVar());
		S.addClause(lit_True);
		assert(in_latches.size());
		start_of_activation_vars = S.nVars();
		start_of_in_latches = in_latches[0];
		for(int i = 0;i<in_latches.size();i++){
			if(in_latches[i]<start_of_in_latches)
				start_of_in_latches = in_latches[i];
		}
		start_of_out_latches=out_latches[0];
		for(int i = 0;i<out_latches.size();i++){
			if(out_latches[i]<start_of_out_latches)
				start_of_out_latches = out_latches[i];
		}
		for(int i = 0;i<in_latches.size();i++){
			Var v =in_latches[i];
			in_latch_to_index.growTo(v-start_of_in_latches+1,-1);
			in_latch_to_index[v-start_of_in_latches]=i;
		}
		for(int i = 0;i<out_latches.size();i++){
			Var v =out_latches[i];
			out_latch_to_index.growTo(v-start_of_out_latches+1,-1);
			out_latch_to_index[v-start_of_out_latches]=i;
		}

		if(opt_ternary){
			ternary = new Ternary(mgr,&priority);
		}

		seen_subsume.growTo(in_latches.size()*2);
		priority.growTo(in_latches.size());
		frames.push();



		vec<Lit> t;
		for(int i = 0;i<in_latches.size();i++){
			t.clear();
			lbool r = resets[i];
			if(r==l_True){
				t.push(mkLit(in_latches[i]));
				addNewClause(0,t);
			}else if(r==l_False){
				t.push(~mkLit(in_latches[i]));
				addNewClause(0,t);
			}
		}
		frames.push();


	}
	int depth(){
		return frames.size()- 1;
	}
	vec<int> tmp_seen;
	void printTCube(TCube * t, bool force_print=false, bool print_input=true){
			if(!force_print )
				return;
			printf("TCube %d [", t->frame);
			printLatches(t->assignment,true,force_print);
			printf(" PI: ");
			printInput(t->primary_inputs,force_print);
			printf("]");
		}
		void printInput(vec<Lit> & input, bool force_print=false){
				if(!force_print )
							return;
				tmp_seen.clear();

				for(int i = 0;i<input.size();i++){
					Lit l = input[i];
					Var v = var(l);
					tmp_seen.growTo(v+1,0);

					if(sign(l)){
						tmp_seen[var(l)]=-1;
					}else{
						tmp_seen[var(l)]=1;
					}

				}

				for(int i = 0;i<primary_inputs.size();i++){
					Var v = primary_inputs[i];
					if(v>=tmp_seen.size())
						printf("x");
					else{
						if(tmp_seen[v]==1)
							printf("1");
						else if (tmp_seen[v]==-1){
							printf("0");
						}else{
							printf("x");
						}
					}
				}
			}
		void printLatches(vec<Lit> & latches,bool out, bool force_print=false){
			if(!force_print)
						return;
			tmp_seen.clear();
			tmp_seen.growTo(out_latches.size());
			for(int i = 0;i<latches.size();i++){
				Lit l = latches[i];

				if(out){
					assert(isOutLatch(l));
					if(sign(l)){
						tmp_seen[outLatchN(var(l))]=-1;
					}else{
						tmp_seen[outLatchN(var(l))]=1;
					}
				}else{
					assert(isInLatch(l));
					if(sign(l)){
						tmp_seen[inLatchN(var(l))]=-1;
					}else{
						tmp_seen[inLatchN(var(l))]=1;
					}
				}
			}

			for(int i = 0;i<out_latches.size();i++){
				if(tmp_seen[i]==1)
					printf("1");
				else if (tmp_seen[i]==-1){
					printf("0");
				}else{
					printf("x");
				}
			}
		}
	void printFrameSizes(){
		if(opt_verb<2)
			return;
		for(int i = 0;i<frames.size();i++){
			printf("%d ",frames[i].size());
		}
		printf("\n");
	}
	void printFrames(bool details=false){
		if(opt_verb<2)
			return;
		printFrameSizes();
		printf("---------------\n");
		for(int i = 0;i<frames.size();i++){
			printf("%d: ",i);
			vec<Lit> & frame = frames[i];
			for(int j = 0;j<frame.size();j++){
				Lit act = frame[j];

				int clauseN =getClauseFromActivation(act);
				if(!is_subsumed[clauseN]){
					if(!details){
						printf("%d, ",clauseN);
					}else{
						printf("%d [",clauseN);
						printLatches(clauses[clauseN],false);
						printf("], ");
					}
				}
			}
			printf("\n");
		}
	}
	int main_iteration=0;
	bool solve(vec<Lit> & property,vec<int> & aiger_property,bool property_needs_preimage = true){

		if(opt_coi){
			if(aiger_property.size()!=property.size()){
				throw std::runtime_error("If coi is used, then aiger lits must be supplied for the bad state\n");
			}
			buildCOI(aiger_property);
		}
		vec<Lit> empty;
		while(true){
			while(pre_image(property,aiger_property,pre,pre_inputs,property_needs_preimage)){
				++main_iteration;
				inToOut(pre);
				assert(hasOutLatches(pre));
				TCube * B = new TCube(depth(),property_needs_preimage? pre:property,property_needs_preimage? pre_inputs:empty,nullptr);
				B->ref();
				if(opt_verb>=2){
					printf("Blocking ");
					printTCube(B); printf("\n");
				}
				TCube * cex = nullptr;
				if((cex =block_cube(B))!=nullptr){
					//found cex
					checkCEX(property,cex);
					printCEX(cex);
					return true;
				}
				bool r = B->deref();
				assert(!r);
				delete(B);
				printFrames(true);
				assert(TCube::remainingTCubes()==0);
			}
			assert(TCube::remainingTCubes()==0);
			frames.push();
			printFrames();
			if(propagateClauses()){
				printFrames();
				checkInvariant(property);
				return false;//found inductive proof
			}
			printFrames();
		}

		//unreachable
		assert(false);
		return true;
	}



/*	bool solve(vec<Lit> & bad_state){

		//}
		//unreachable
		assert(false);
		return true;
	}*/
	vec<bool> latch_in_block;

	int iter=0;
	//I will assume the bad state is a cube over the latches, rather than arbitrary combinatorial logic.
	TCube * block_cube(TCube * B){

		vec<Lit> assign;
		vec<Lit> block;


		assert(Q.size()==0);
		B->ref();
		Q.insert(B->frame,B);

		//Now, block all the cubes that have been found, starting with the earliest time frames
		while( Q.size()){
			if(++iter==180){
				int a=1;
			}
			printFrames(true);
			TCube & s =* Q.removeMin();
			if(s.frame==0){
				return &s;//cex
			}
			assert(hasOutLatches(s.assignment));
			if(isReset(s.assignment))
				return &s;//is this actually reachable? I'm not sure.

			int blocked_at = depth();
			if(!isBlocked(s.frame,s.assignment, blocked_at))
			{
				//Note that we omit the extra 'semantic' check from the PDR paper.
				//This should probably be added back in.

				assert(!isReset(s.assignment));//because if it was the reset state, we'd have already found a counter example
				assign.clear();
			/*	for(int i = 0;i< s.assignment.size();i++){
					Lit in = s.assignment[i];
					assign.push(inToOut(in)); //solvers[0]->fromSuper(in));
				}*/
				assert(s.frame>0);
				if(solveAtFrame(s.assignment,s.frame-1)){
					//keep this bad state in the queue, as it still needs to be blocked, and add a new one corresponding to the solution we just found
					Q.insert(s.frame,&s);
					TCube * t = new TCube(s.frame-1,&s);
					t->ref();
					for(int i = 0;i<in_latches.size();i++){
						lbool v = S.modelValue(in_latches[i]);
						if(v==l_True){
							Lit l = mkLit(in_latches[i]);
							t->assignment.push(inToOut(l));
						}else if (v==l_False){
							Lit l = ~mkLit(in_latches[i]);
							t->assignment.push(inToOut(l));
						}
					}
					for(int i =0;i<primary_inputs.size();i++){
						if(S.modelValue(primary_inputs[i])==l_True){
							t->primary_inputs.push(mkLit(primary_inputs[i]));
						}else{
							t->primary_inputs.push(~mkLit(primary_inputs[i]));
						}
					}
					assert(t->assignment.size());
					//remove literals that are outside of the cone of influence from the solution
					//note: how does this interact with aiger constraints?
					 if(opt_coi){
						 cur_coi.zero();
						for(int i = 0;i<s.assignment.size();i++){
							Lit l = s.assignment[i];
							int latchN = outLatchN(var(l));
							cur_coi.Or(latch_coi[latchN]);//should this take into account the sign of the output?
						}
						int i,j=0;
						for(i = 0;i< t->assignment.size();i++){
							Lit l = t->assignment[i];
							int latchN = outLatchN(var(l));
							if(cur_coi[i]){
								t->assignment[j++]=t->assignment[i];
							}else{
								stats_coi_lits_removed++;
							}
						}
						t->assignment.shrink(i-j);
					 }

					 if(ternary){
						 ternary_assign.clear();
						 ternary_assign.growTo(in_latches.size(),l_Undef);
						ternary_preserve.clear();
						for(int i = 0;i<s.assignment.size();i++){
							Lit l = s.assignment[i];
							int latchN = outLatchN(var(l));
							ternary_preserve.push(mgr->latches[latchN].next);
						}
						for(int i = 0;i<t->assignment.size();i++){
							Lit l = t->assignment[i];
							int latchN = outLatchN(var(l));
							if(sign(l)){
								ternary_assign[latchN]=l_False;
							}else{
								ternary_assign[latchN]=l_True;
							}
						}

						ternary_inputs.clear();
						for(int i = 0;i<primary_inputs.size();i++){
							Var v = primary_inputs[i];
							ternary_inputs.push(S.modelValue(v));
						}


						ternary->reduce(ternary_assign,ternary_inputs,ternary_preserve);
						int i,j=0;
						for(i = 0;i<t->assignment.size();i++){
							Lit l = t->assignment[i];
							int latch = outLatchN(var(l));
							lbool v= ternary_assign[latch];
							if(v!=l_Undef){
								t->assignment[j++]=l;
							}else{
								stats_ternary_lits_removed++;
							}
						}
						t->assignment.shrink(i-j);
					 }
					Q.insert(t->frame,t);
				}else{

					//assignment s.cube was blocked by the interpolant at s.frame
					//cube is already generalized by the solver
					block.clear();
					int min_frame=depth();

					for(int i = 0;i<S.conflict.size();i++){
						Lit l = S.conflict[i];
						if(isOutLatch(l)){
							block.push(outToIn(l));
						}else{
							assert(isActivationLit(l));
							int clauseN = getClauseFromActivation(l);
							int f = getClauseFrame(clauseN)+1;
							assert(f>=s.frame);
							//following PDR, see if the conflict clause actually corresponds to a later frame
							if(f<min_frame){
								min_frame=f;
							}
						}
					}
					assert(min_frame>=s.frame);
					assert(hasInLatches(block));
					if(conflictsWithReset(block,true)){
						//we implicitly removed literals above, by only using latches present in the conflict set.
						//however, if the remaining literals intersect with the reset state, then we need to weaken the clause until it is an overapprox of the reset state
						//we can do this by adding the negation of a literal from the original assignment
						latch_in_block.clear();
						latch_in_block.growTo(in_latches.size());
						for(int i = 0;i<block.size();i++){
							Lit l = block[i];
							assert(isInLatch(l));
							int latchN = inLatchN(var(l));
							latch_in_block[latchN]=true;
						}
						for(int i = 0;i<s.assignment.size();i++){
							Lit l = s.assignment[i];
							assert(isOutLatch(l));
							if(satisfiesReset(outToIn(~l))){
								//only consider literals from the assignment where, when negated, they satisfy the reset state
								int latchN = outLatchN(var(l));
								if(!latch_in_block[latchN]){
									block.push(~outToIn(l));
									break;
								}
							}
						}

					}

					//In addition to collecting clauses found by the modular SAT solver internally, also add this outer conflict clause.
					addNewClause(min_frame, block,false);

					//optionally add the proof obligation to the subsequent time frame
					if(opt_queue_forward && (opt_keep_all_cubes || s.frame < depth())){
					   s.frame++;
					   Q.insert(s.frame,&s);
					}else{
						if(!s.deref())
							delete(&s);
					}

				}
			}else{
				assert(blocked_at>=s.frame);
				//Keep syntactically blocked cubes to be tested at future time frames, instead of discarding them
				if( opt_queue_forward  && (opt_keep_all_cubes || blocked_at < depth())){
					//blocked_at is a small optimization; instead of just queueing these forward to s.frame+1,
					//we can detect the highest frame that the cube would be blocked at (which must be >= s.frame) cheaply in the isBlocked function, and enqueue the cube after that frame.
					//This avoids repeated isBlocked tests for the same cube.
				   s.frame=blocked_at+1;
				   Q.insert(s.frame,&s);
				}else{
					if(!s.deref())
						delete(&s);
				}

			}
		}



		return nullptr;
	}
	vec<Lit> tmp_c;
	int gen_iter =0 ;
	/**
	 * Add a new clause to the frame constraints, generalizing it in the process.
	 */
	void addNewClause(int f, vec<Lit> & cube, bool already_in_solver=false){
		if(++gen_iter==148){
			int a=1;
		}
		assert(hasInLatches(cube));
		assert(cube.size());
		assert(!isSubsumedAt(cube,f));
		assert(!conflictsWithReset(cube,true));

		int old_size = cube.size();
		bool generalized = false;
		if(opt_generalize_rounds>0){
			for(int i = 0;i<opt_generalize_rounds;i++){
				generalized=generalize(cube, f-1);
				if(!generalized)
					break;
			}
			 // set priority flops
			for (int i = 0; i < cube.size(); i++ )
			{
				priority[inLatchN(var(cube[i]))]++;
			}

		}
		stats_generalize_lits_removed+=old_size-cube.size();
		assert(!conflictsWithReset(cube,true));


		//Immediately try to propagate the clause forward as far as possible (PDR does this)
		Lit act = mkLit(S.newVar());
		assert(hasInLatches(cube));
		cube.push(~act);
		S.addClause(cube);

		cube.pop();
		int max = f;
		if(opt_early_clause_prop){
			tmp_c.clear();
			tmp_c.push(act);
			for (int k = 0;k<cube.size();k++){
				tmp_c.push( ~inToOut(cube[k]));
			}
			for(;f<depth()-1;f++){

				//check if blocked is also stopped by later time frame
				//this should really temporarily add ~assign to the solver.
				if(solveAtFrame(tmp_c,f)){
					max=f;
					break;
				}else{
					max = f+1;
				}
			}
		}
		if(max>depth())
			max = depth();
		int index = var(act)- start_of_activation_vars;
		int clauseN = clauses.size();


		lit_to_clause.growTo(index+1,-1);
		lit_to_clause[index]=clauseN;
		clauses.push();
		assert(hasInLatches(cube));
		cube.copyTo(clauses.last());
		abstractions.growTo(clauseN+1);
		clause_frame.growTo(clauseN+1,-1);
		is_subsumed.growTo(clauseN+1);
		abstractions[clauseN]=calcAbstraction(cube);
		clause_frame[clauseN]=max;
		if(opt_early_subsumption){
			markSubsumed(cube,abstractions[clauseN],max,0);
		}

		//immediately see if act subsumes anything
		frames[max].push(act);
		if(opt_verb>=2){
			printf("Adding clause to %d: [",max);
			printLatches(cube,false);
			printf("], removed %d lits through gen (%d)\n",old_size-cube.size(),stats_generalize_lits_removed);
		}

		tmp_c.clear();
	}
	vec<Lit> sub_t;
	bool isSubsumedAt(vec<Lit> & cube, int frame){
		if(frame==0)
			return false;//hack
		assert(hasInLatches(cube));
		sub_t.clear();
		for(int i = 0;i<cube.size();i++){
			sub_t.push(~inToOut(cube[i]));
		}

		int ignore;

		return isBlocked(frame,sub_t,ignore);
	}
	/**
	 * Apply clause propagation.
	 */
	bool propagateClauses(){
		static int p_iter=0;
		++p_iter;
		//we can probably safely start at t=1 after the first run through; and the benefit to ever starting at frame 0 is unclear... it just means we force the checker to attempt to check for any constant latches.
		for(int f = 0;f<depth();f++){

			printFrames();

			vec<Lit> & frame = frames[f];//activation lits for this frame
			if(frame.size()==0){
				moveInvariantToEnd(f);
				return true;//invariant found
			}
			sort_by_size(frame);//sort the interpolant by size (so we can ensure that a clause cannot subsume an earlier one, ignore equal sized clauses which we assume to be unique )

			int j,i=0;
			for(j = 0;j<frame.size();j++){

				//first remove any subsumed clauses from this time frame
				Lit l = frame[j];
				int clauseN = getClauseFromActivation(l);
				if(is_subsumed[clauseN]){
					assert(isSubsumedAt(clauses[clauseN],f));
					releaseClause(l);
					frame[j]=lit_True;
					continue;
				}

				assert(clause_frame[clauseN]==f);
				vec<Lit> & c = clauses[clauseN];
				if(opt_remove_subsumed_during_clause_prop)//PDR also does this - but it really shouldn't be necessary, if subsumption was already checked when the clause was added to this frame
					markSubsumed(c,abstractions[clauseN],f,j+1);
				tmp_c.clear();

				for (int k = 0;k<c.size();k++){
					Lit l = ~inToOut(c[k]);
					tmp_c.push(l);
				}

				if(!solveAtFrame(tmp_c,f)){


					//PDR does this, but I'm not clear on why this extra subsumption check is necessary (as it should happen anyhow at the next iteration)
					if(opt_clause_prop_subsume)
						markSubsumed(c,abstractions[clauseN],f+1);

					//push this clause forward to the next time frame
					frames[f+1].push(l);
					assert(clause_frame[clauseN]==f);
					clause_frame[clauseN]=f+1;
				}else{

					//keep this clause in the current time frame
					frame[i++]=frame[j];
				}

			}
			frame.shrink(j-i);

			if(frame.size()==0){
				moveInvariantToEnd(f);
				return true;//invariant found
			}
		}
		printFrames(true);
		if(opt_early_subsumption || opt_remove_subsumed_during_clause_prop || opt_remove_subsumed_last ||opt_clause_prop_subsume){
			vec<Lit> & frame = frames.last();//activation lits for this frame
			sort_by_size(frame);
			//check each entry in the last time frame for subsumption
			int i, j = 0;

			for(i = 0;i<frame.size();i++){
				Lit l = frame[i];
				int clauseN = getClauseFromActivation(l);
				vec<Lit> & c = clauses[clauseN];
				if(is_subsumed[clauseN]){
					assert(isSubsumedAt(clauses[clauseN],depth()));
					frame[j]=lit_True;
					releaseClause(l);
					continue;
				}
				//An optional, final round of subsumption.
				if(opt_remove_subsumed_last)
					markSubsumed(c,abstractions[clauseN],depth(),i+1);
				frame[j++]=frame[i];
			}
			frame.shrink(i-j);
		}
		return false;
	}
	struct LessThanClause{
		IC3 * outer;

		bool operator () (const Lit & x, const Lit & y){
			int n1 = outer->getClauseFromActivation(x);
			int n2 = outer->getClauseFromActivation(y);
			vec<Lit> & c1 = outer->clauses[n1];
			vec<Lit> & c2 = outer->clauses[n2];
			return c1.size()<c2.size();
		}

		LessThanClause(IC3 * outer):outer(outer){	}
	};

	void sort_by_size(vec<Lit> & clauses){
		sort(clauses,LessThanClause(this));
	}


	//Remove subsumed clauses from the interpolant at a frame, starting from entry 'start'.
	void markSubsumed(vec<Lit> & gen,uint32_t abstraction, int frame, int start=0){
		assert(hasInLatches(gen));
		assert(gen.size());
		assert(seen_subsume.size()>0);
		for(int i = 0;i<seen_subsume.size();i++)
			seen_subsume[i]=false;



		for(int j = 0;j<gen.size();j++){
			Lit l = gen[j];
			int p = inLatchN(var(l))*2+sign(l);
			assert(p<seen_subsume.size());
			 seen_subsume[p]=true;

		}


		int j, h = start;
		for(j=start;j<frames[frame].size();j++){
			Lit act = frames[frame][j];
			int clauseN = getClauseFromActivation(act);
			vec<Lit> & c = clauses[clauseN];

			bool subsumed= is_subsumed[clauseN];
			if(!subsumed &&  c.size()>gen.size()){
				assert(gen.size());
				if( ((abstraction & ~abstractions[clauseN]) == 0)) {
					int count = 0;
					for(int k = 0;k<c.size();k++){// && (gen.size()- count > (c.size()-k) )
						Lit l = c[k];
						int p = inLatchN(var(l))*2+sign(l);
						assert(p<seen_subsume.size());
						if(seen_subsume[p]){
							count++;
						}
					}
					assert(count<=gen.size());
					subsumed= count==gen.size();
					if(subsumed){
						stats_subsumed_clauses_removed++;
						is_subsumed[clauseN]=true;//mark this clause as subsumed.
					}
					//cannot release the variable yet; that will happen later.
				}
			}

		}

	}
	/**
	 * Check if a clause is SYNTACTICALLY blocked, as in ABC's PDR implementation.
	 */
	bool isBlocked(int frame, vec<Lit> & assignment, int & blocked_at){
		blocked_at=frame;
		assert(hasOutLatches(assignment));
		if(!opt_check_blocked || frame>depth())//need to change this to > if we allow frame_infinity
			return false;

		static vec<Lit> check;
		static vec<Lit> cube;
		check.clear();
		for(int i = 0;i<assignment.size();i++){
			check.push(~outToIn(assignment[i]));
		}
		if(opt_verb>2){
			printLatches(check,false);printf("\n");
		}
		assert(seen_subsume.size()>0);
		for(int i = 0;i<seen_subsume.size();i++)
			seen_subsume[i]=false;


			uint32_t abstraction=0;
			for(int i = 0;i<check.size();i++){
				Lit l = check[i];
				int p = inLatchN(var(l))*2+sign(l);
				assert(p<seen_subsume.size());
				 seen_subsume[p]=true;
				 abstraction |= 1 << (var(l) & 31);
				 if(S.value(l)==l_True)
					 return true;
			}

			assert(abstraction == calcAbstraction(check));


			for(int f = frames.size()-1;f>=frame;f--){

				vec<Lit> & frame= frames[f];
				for(int i = 0;i<frame.size();i++){
					Lit l = frame[i];
					int clauseN = getClauseFromActivation(l);
					vec<Lit> & c = clauses[clauseN];

					if (!is_subsumed[clauseN] && c.size()<=check.size() && (abstractions[clauseN] & ~abstraction) == 0){
						bool subsumes=true;
						for(int j = 0;j<c.size();j++){
							Lit l = c[j];
							int p = inLatchN(var(l))*2+sign(l);
							assert(p<seen_subsume.size());
							if(!seen_subsume[p]){
								subsumes=false;
								break;
							}
						}
						if(subsumes){
							blocked_at=f;
							return true;
						}
					}
				}
			}
			return false;
	}







	//----------generalization----------

	vec<int> order;
	vec<Lit> tmp_clause;
	vec<Lit> tmp_asummptions;
	vec<bool> lits_to_keep;
	 vec<int> gen_order;
	bool generalize(vec<Lit> & in_clause, int frame){
		if(in_clause.size()==1)
			return false;
		static int gen_it=0;
		if(++gen_it==31){
			int a=1;
		}
		assert(hasInLatches(in_clause));
		int initial_size = in_clause.size();

		gen_order.clear();
		if(opt_sort_when_generalizing){
			 sortByPriority(in_clause, gen_order,frame );
		}else{
			for(int i = 0;i<in_clause.size();i++)
				gen_order.push(i);
		}

		vec<Lit> & out_clause = tmp_clause;
		out_clause.clear();

		lits_to_keep.clear();
		lits_to_keep.growTo(out_latches.size());

		int num_sat_reset=0;
		for (int j = 0; j < in_clause.size(); j++ )
		{
			Lit l = in_clause[j];
			num_sat_reset+= satisfiesReset(l);
			out_clause.push(~inToOut(l));  //mkLit(var(clause[j])+offset,sign(clause[j]));
		}

		//there is no need to add not s to frame 0, since the inputs are fully constrained
		assert(gen_order.size()==in_clause.size());

		Lit activation  =lit_Undef;
		if(frame>0){
			//the 0th frame doesn't need the clause added
			activation = mkLit(S.newVar());
		}

		for (int j = 0; j < gen_order.size(); j++ )
		{
			// use ordering
	//        i = j;
			int index = gen_order[j];
			assert(!conflictsWithReset(out_clause));

			// try removing this literal
			Lit in_l = in_clause[index];
			Lit out_l = out_clause[index];
			if(out_l==lit_True)
				continue;

	#ifndef DEBUG_IC3
			int tcount = 0;
			for(int k = 0;k<out_clause.size();k++){
				if(out_clause[k] != lit_True  && satisfiesReset(~outToIn(out_clause[k])))
					tcount++;
			}
			assert(tcount==num_sat_reset);
	#endif
			assert(num_sat_reset>=1);
			if(satisfiesReset(in_l) && num_sat_reset==1){
				continue;//can't remove this without intersecting reset
			}
			assert(!conflictsWithReset(out_clause));

			assert(in_clause[index]==~outToIn(out_l));
			out_clause[index]=lit_True;
			in_clause[index]=~lit_True;
			assert(out_clause.size()==in_clause.size());
			assert(!conflictsWithReset(out_clause));

			if(frame){
				in_clause.push(~activation);
				S.addClause(in_clause);
				out_clause.push(activation);
			}

			if(solveAtFrame(out_clause,frame)){
				out_clause[index]=out_l;
				in_clause[index]=in_l;

				if(frame){
					S.releaseVar(~activation);
					activation= mkLit(S.newVar());
					in_clause.pop();
					out_clause.pop();
				}

				continue;
			}

			if(frame){
				in_clause.pop();
				out_clause.pop();
			}
			assert(out_clause.size()==in_clause.size());


			assert(!conflictsWithReset(out_clause));
			if(satisfiesReset(in_l))
				num_sat_reset--;
			for(int j =0;j<S.conflict.size();j++){
				Lit a = S.conflict[j];

				if(isOutLatch(a)){
					int latch = outLatchN(var(a));
					assert(!lits_to_keep[latch]);
					assert(latch != outLatchN(var(out_l)));
					lits_to_keep[latch]=true;
				}
			}

			for(int i = 0;i<out_clause.size();i++){
				Lit l = out_clause[i];
				if(l!=lit_True){
					int latch = outLatchN(var(l));
					if(lits_to_keep[latch]){
						lits_to_keep[latch]=false;
					}else{
						Lit in_l = ~outToIn(l);
						if(satisfiesReset(in_l)){
							if(num_sat_reset==1){
								continue;//cannot drop this literal without conflict with reset
							}else{
								num_sat_reset--;
							}
						}
						assert(num_sat_reset>=1);
						out_clause[i]=lit_True;
						in_clause[i]=lit_True;
						assert(!conflictsWithReset(out_clause));
					}
				}
			}

			assert(!conflictsWithReset(out_clause));

	#ifndef DEBUG_IC3
			for(int j = 0;j<lits_to_keep.size();j++)
					assert(!lits_to_keep[j]);
			int d_num_latches_true=0;
			for (int j = 0; j < out_clause.size(); j++ )
				d_num_latches_true+=( (out_clause[j] != lit_True) && satisfiesReset(~outToIn(out_clause[j])));
			assert(num_sat_reset==d_num_latches_true);
	#endif
		}


		num_gen++;



		if(activation!=lit_Undef){
			S.releaseVar(~activation);
		}

		assert(!conflictsWithReset(out_clause));
		//ok, now drop all the undef's from the lit
		int i,j = 0;
		for(i = 0;i<out_clause.size();i++){
			if(out_clause[i]!=lit_True)
				in_clause[j++]=~outToIn(out_clause[i]); //mkLit(var(out_clause[i])-offset,sign(out_clause[i]));
		}
		in_clause.shrink(i-j);



		return in_clause.size()< initial_size;

	}


	void sortByPriority(const vec<Lit> & cube, vec<int> &order, int frame)
	{

		order.clear();
	    // initialize variable order
	    for (int i = 0; i < cube.size(); i++ )
	    	order.push(i);

	    for (int i = 0; i < cube.size()-1; i++ )
	    {
	    	//TODO: fix this!
	        int best_i = i;
	        for (int j = i+1; j < cube.size(); j++ ){
					if(priority[inLatchN(var(cube[j]))] < priority[inLatchN(var(cube[best_i]))])
						 best_i = j;
	        }

	        int temp = order[i];
	        order[i] = order[best_i];
	        order[best_i] = temp;
	    }


	}
	bool conflictsWithReset(vec<Lit> & cube, bool inputs=false){
		if(!inputs){

			//since reset state is all latches false, we conflict if we aren't assigning at least one latch to true
			for(int i = 0;i<cube.size();i++){
				Lit l = cube[i];
				if(l!=lit_Undef && l!=lit_True && satisfiesReset(outToIn(~l)))//!sign(cube[i]))
					return false;
			}
		}else{
			assert(hasInLatches(cube));
			//since reset state is all latches false, we conflict if we aren't assigning at least one latch to false
			for(int i = 0;i<cube.size();i++){
				Lit l = cube[i];
				if(satisfiesReset(l))
					return false;
			}
		}
		return true;
	}

	bool isReset(vec<Lit> & assignment, bool inputs=false){
		for(int i = 0;i<assignment.size();i++){
			Lit l =assignment[i];
			int latch = inputs?inLatchN(var(l)):outLatchN(var(l));
			lbool reset = resets[latch];
			if(reset!=l_Undef && (sign(l)!=(reset==l_False))){
				return false;
			}
		}
		return true;
	}
	void printCEX(TCube * c){

			if(!c){
				printf("No counter example\n");
				return;
			}
			printf("Counter example:\n");
			while(true){
				{
					printTCube(c,true);printf("\n");
					TCube * parent = c->parent;
					if(!parent)
						break;
					c = parent;
				}
			}
		}
	void checkCEX(vec<Lit> & p,TCube * cex){
		if (opt_verify){
			if(!isReset(cex->assignment)){
				throw std::runtime_error("Error in ic3; bad counter example!");
			}
			vec<Lit> tmp;
			while(true){
				if(cex->parent==nullptr){
					tmp.clear();
					assert(cex->refs>0);
					cex->assignment.copyTo(tmp);
					outToIn(tmp);
					if(!isPreimage(tmp,p)){
						throw std::runtime_error("Error in ic3; bad counter example!");
					}
					return;
				}else{
					tmp.clear();
					TCube * parent = cex->parent;
					assert(cex->refs>0);
					cex->assignment.copyTo(tmp);
					outToIn(tmp);
					if(!isPreimage(tmp, parent->assignment)){
						throw std::runtime_error("Error in ic3; bad counter example!");
					}
					cex = parent;
				}
			}
		}
	}

	void checkInvariant(vec<Lit> & p){
		if (opt_verify){
			bool r = solveAtFrame(p,depth());
			if(r){
				throw std::runtime_error("Error in ic3; bad proof!");
			}
			static vec<Lit> tmp;

			vec<Lit> & frame =frames.last();
			for(int j = 0;j<frame.size();j++){
				Lit l = frame[j];
				assert(isActivationLit(l));
				int clauseN = getClauseFromActivation(l);

				vec<Lit> & c = clauses[clauseN];
				assert(c.size());
				assert(hasInLatches(c));

				tmp.clear();
				for(int k = 0;k<c.size();k++){
					tmp.push(~inToOut(c[k]));
				}
				if(solveAtFrame(tmp,depth())){
					throw std::runtime_error("Error in ic3; bad proof!");
				}

			}

		}
	}


	template<class Lits>
	uint32_t calcAbstraction(const Lits & ls) {

	    uint32_t abstraction = 0;
	    for (int i = 0; i < ls.size(); i++)
	        abstraction |= 1 << (var(ls[i]) & 31);
	    return abstraction;
	}

};

};

#endif /* IC3_H_ */
