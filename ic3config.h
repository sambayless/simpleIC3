/*
 * ic3config.h
 *
 *  Created on: Jan 3, 2016
 *      Author: sam
 */

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
