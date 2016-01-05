/*
 * ic3config.cpp
 *
 *  Created on: Jan 3, 2016
 *      Author: sam
 */


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
