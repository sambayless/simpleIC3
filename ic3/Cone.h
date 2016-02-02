/*
 * Cone.h
 *
 *  Created on: 2013-04-30
 *      Author: sam
 */

#ifndef CONE_H_
#define CONE_H_
#include <minisat/mtl/Vec.h>

#include "util/Bitset.h"

extern "C" {
#include "aiger/aiger.h"
}

using namespace Minisat;
namespace SimpIC3{
class ConeOfInfluence{
	bool opt_coi=true;
	bool opt_print_coi=false;
	aiger* mgr;

	vec<bool> seen;
public:


	ConeOfInfluence(aiger * _mgr):mgr(_mgr){
		seen.growTo(mgr->maxvar+1);
	}

	void markCone( int l);
	void collectCOI(int from, Bitset & store);
	void setupCOI();
};

};
#endif /* CONE_H_ */
