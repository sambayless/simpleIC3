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

#ifndef IC3TYPES_H_
#define IC3TYPES_H_
#include <cassert>
#include <minisat/core/SolverTypes.h>
#include <minisat/mtl/Vec.h>
namespace SimpIC3{
struct TCube{
	static long remaining_tcubes;
	static long total_tcubes;
	int frame;
	int refs=0;
	TCube * parent;
	Minisat::vec<Minisat::Lit> assignment;
	Minisat::vec<Minisat::Lit> primary_inputs;
	 static long allocatedTCubes(){
		return total_tcubes;
	}
	static long remainingTCubes(){
		return remaining_tcubes;
	}

	TCube():frame(-1),parent(nullptr){
		remaining_tcubes++;total_tcubes++;
	}
	TCube(int _frame,TCube * parent):frame(_frame),parent(parent){
		remaining_tcubes++;total_tcubes++;
		if(parent){
			parent->ref();
		}
	}

	TCube(const TCube & from):frame(from.frame),parent(from.parent){
		remaining_tcubes++;total_tcubes++;
		from.assignment.copyTo(assignment);
		from.primary_inputs.copyTo(primary_inputs);
	}
	TCube(int _frame,const Minisat::vec<Minisat::Lit> & assign,TCube * parent):frame(_frame),parent(parent){
		remaining_tcubes++;total_tcubes++;
		assign.copyTo(assignment);
	}
	TCube(int _frame,const Minisat::vec<Minisat::Lit> & assign,const Minisat::vec<Minisat::Lit> & primary_inputs_assignmnet,TCube * parent):frame(_frame),parent(parent){
		remaining_tcubes++;total_tcubes++;
		assign.copyTo(assignment);
		primary_inputs_assignmnet.copyTo(primary_inputs);
	}

	~TCube(){
		assert(refs==0);
		parent=nullptr;
    	remaining_tcubes--;
	}

	bool deref(){
		refs--;
		assert(refs>=0);
		if(refs==0){
			if(parent && !parent->deref()){
				delete(parent);
			}
		}
		return refs>0;
	}
	void ref(){
		refs++;
	}
};


};
#endif /* IC3TYPES_H_ */
