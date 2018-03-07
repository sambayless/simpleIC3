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
 * Provides basic Cone-of-Influence detection
 */
#ifndef CONE_H_
#define CONE_H_
#include <minisat/mtl/Vec.h>

#include "simpleIC3/util/Bitset.h"

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
