/*
 * IC3Types.h
 *
 *  Created on: Jan 1, 2016
 *      Author: sam
 */

#ifndef IC3TYPES_H_
#define IC3TYPES_H_

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
	}
	TCube(int _frame,const Minisat::vec<Minisat::Lit> & assign,TCube * parent):frame(_frame),parent(parent){
		remaining_tcubes++;total_tcubes++;
		assign.copyTo(assignment);
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
