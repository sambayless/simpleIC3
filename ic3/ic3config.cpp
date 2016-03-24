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

#include "ic3config.h"

using namespace Minisat;
static const char * _cat_ic3 = "IC3";
IntOption SimpIC3::opt_verb(_cat_ic3, "verb", "Verbosity level (0=silent, 1=some, 2=more).", 0, IntRange(0, 3));


IntOption SimpIC3::opt_generalize_rounds(_cat_ic3, "generalize", "Number of rounds of generalziation to apply to each new clause (0 disables).",1, IntRange(0, INT32_MAX));
BoolOption  SimpIC3::opt_coi(_cat_ic3, "coi", "Apply Cone-of-Influence generalization.", true);
BoolOption SimpIC3::opt_ternary(_cat_ic3, "ternary", "Apply ternary simulation.", true);
BoolOption SimpIC3::opt_check_blocked(_cat_ic3, "syntactic-blocking", "Check whether cubes are syntactically blocked before SAT sovling.", true);

BoolOption SimpIC3::opt_queue_forward (_cat_ic3, "queue-forward", "Re-check blocked bad-states at later frames.", true);


BoolOption SimpIC3::opt_keep_all_cubes (_cat_ic3, "queue-forward-all", "Never discard bad-states (always re-check them).", false);
BoolOption SimpIC3::opt_early_clause_prop(_cat_ic3, "early-clause-prop", "Apply clause propagation to new clauses as soon as they are created.", true);
BoolOption SimpIC3::opt_early_subsumption(_cat_ic3, "early-subsumption", "Check whether each new clause subsumes any existing clauses", true);
BoolOption SimpIC3::opt_verify(_cat_ic3, "verify", "Double-check witness or proof for correctness", true);;
BoolOption SimpIC3::opt_remove_subsumed_during_clause_prop(_cat_ic3, "subsume-during", "Remove subsumed clauses during clause propagation.", false);;
BoolOption SimpIC3::opt_remove_subsumed_last(_cat_ic3, "subsume-last", "Re-check blocked bad-states at later frames.", true);
BoolOption SimpIC3::opt_clause_prop_subsume(_cat_ic3, "subsume", "Check whether each propagated clause subsumes any existing clauses at its new frame", true);
BoolOption SimpIC3::opt_sort_when_generalizing(_cat_ic3, "sort-gen", "Sort literals heuristically during generalization.", true);
