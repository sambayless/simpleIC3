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
#ifndef IC3CONFIG_H_
#define IC3CONFIG_H_

#include <minisat/utils/System.h>

#include <minisat/utils/Options.h>
namespace SimpIC3{
extern Minisat::IntOption  opt_verb;
extern Minisat::IntOption  opt_generalize_rounds;
extern Minisat::BoolOption  opt_coi;
extern Minisat::BoolOption   opt_ternary ;
extern Minisat::BoolOption   opt_check_blocked ;

extern Minisat::BoolOption   opt_queue_forward ;

extern Minisat::BoolOption   opt_forward_all;
extern Minisat::BoolOption   opt_keep_all_cubes;
extern Minisat::BoolOption   opt_early_clause_prop ;
extern Minisat::BoolOption   opt_early_subsumption;
extern Minisat::BoolOption   opt_verify ;
extern Minisat::BoolOption   opt_remove_subsumed_during_clause_prop;
extern Minisat::BoolOption   opt_remove_subsumed_last;
extern Minisat::BoolOption   opt_clause_prop_subsume;
extern Minisat::BoolOption   opt_sort_when_generalizing;
};

#endif /* IC3CONFIG_H_ */
