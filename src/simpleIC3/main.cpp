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
extern "C" {
#include "aiger/aiger.h"
}
#include "simpleIC3/ic3/ic3.h"
#include <minisat/mtl/Vec.h>
#include <minisat/core/SolverTypes.h>
#include "simpleIC3/aiglib/aigtocnf.h"

#include <zlib.h>
#include <errno.h>
#include <minisat/utils/System.h>
#include <minisat/utils/ParseUtils.h>
#include <minisat/utils/Options.h>

using namespace Minisat;
using namespace SimpIC3;
using namespace aiglib;




int main(int argc,char** argv){
	 setUsageHelp("USAGE: %s [options] <input-file>\n\n Where input-file is in AIGER binary format\n");
		setX86FPUPrecision();

		// Extra options:
		//
		IntOption    verb   ("MAIN", "verb",   "Verbosity level (0=silent, 1=some, 2=more).", 1, IntRange(0, 2));
		BoolOption   pre    ("MAIN", "pre",    "Completely turn on/off any preprocessing.", true);


		IntOption    cpu_lim("MAIN", "cpu-lim","Limit on CPU time allowed in seconds.\n", 0, IntRange(0, INT32_MAX));
		IntOption    mem_lim("MAIN", "mem-lim","Limit on memory usage in megabytes.\n", 0, IntRange(0, INT32_MAX));
		IntOption    opt_pi   ("IC3", "pi",   "Property (or bad state) index to check", 0, IntRange(0, INT32_MAX));

		parseOptions(argc, argv, true);

	  aiger *aiger;
	  int property = opt_pi;
	 aiger = aiger_init ();
	 const char *error;
	 if (argc >= 2)
	   error = aiger_open_and_read_from_file (aiger, argv[1]);
	 else
	   error = aiger_read_from_file (aiger, stdin);

	 if (error)
	   {
		 fprintf (stderr,
		   "***  %s: %s\n",
		   (argc >= 1) ? (argv[1]) : "<stdin>", error);
		 exit(1);
	   }


	SimpSolver S;
	Minisat::vec<Minisat::Lit> in_latches;
	Minisat::vec<Minisat::Lit> out_latches;
	Minisat::vec<Minisat::lbool> resets;
	Minisat::vec<Minisat::Lit> outputLits;
	Minisat::vec<Minisat::Lit> badStates;
	Minisat::vec<Minisat::Lit> inputLits;
	Lit lit_True= aigtocnf(S,aiger, inputLits, in_latches,resets,out_latches,outputLits,badStates);
	S.freezeVar(var(lit_True));
	for(int i = 0;i<inputLits.size();i++)
		S.freezeVar(var(inputLits[i]));
	for(int i = 0;i<in_latches.size();i++)
		S.freezeVar(var(in_latches[i]));
	for(int i = 0;i<out_latches.size();i++)
		S.freezeVar(var(out_latches[i]));
	for(int i = 0;i<outputLits.size();i++)
		S.freezeVar(var(outputLits[i]));
	for(int i = 0;i<badStates.size();i++)
		S.freezeVar(var(badStates[i]));
	bool check = S.solve(true,true);
	assert(check);

	Minisat::vec<Minisat::Var> in_latch_vars;
	Minisat::vec<Minisat::Var> out_latch_vars;
	Minisat::vec<Minisat::Var> primary_input_vars;
	//ic3 needs all input and output lits to be positive and unsigned.
	//so create new ones here, just in case any next lits are signed.
	for(int i = 0;i<in_latches.size();i++){
		Lit l = in_latches[i];
		Lit nv = mkLit( S.newVar());
		S.addClause(l,~nv);
		S.addClause(~l,nv);
		in_latch_vars.push(var(nv));
	}
	for(int i = 0;i<out_latches.size();i++){
		Lit l = out_latches[i];
		Lit nv = mkLit( S.newVar());
		S.addClause(l,~nv);
		S.addClause(~l,nv);
		out_latch_vars.push(var(nv));
	}
	for(int i = 0;i<inputLits.size();i++){
		Lit l = inputLits[i];
		Lit nv = mkLit( S.newVar());
		S.addClause(l,~nv);
		S.addClause(~l,nv);
		primary_input_vars.push(var(nv));
	}
	vec<Lit> properties;
	vec<int> aiger_properties;
	if(badStates.size()==0){
		fprintf(stderr,"No bad states, so assuming outputs are badstates\n");
		outputLits.copyTo(properties);
		for(int i = 0;i<aiger->num_outputs;i++)
			aiger_properties.push(aiger->outputs[i].lit);
	}else{
		badStates.copyTo(properties);
		for(int i = 0;i<aiger->num_bad;i++)
			aiger_properties.push(aiger->bad[i].lit);
	}
	if(property>=properties.size()){
		fprintf(stderr,"Property %d is out of range!\n",property);
		exit(1);
	}
	assert(in_latch_vars.size()==out_latch_vars.size());
	int aiger_prop = aiger_properties[property];
	Lit p = properties[property];
	if(in_latch_vars.size()==0){
		fprintf(stderr,"No latches, solving with SAT solver\n");
		if(S.solve(p)){
			exit(10);
		}else{
			exit(20);
		}
	}

	IC3 ic3(S,aiger,in_latch_vars,out_latch_vars, resets,primary_input_vars);



	vec<Lit> tmp;
	tmp.push(p);
	vec<int> atmp;
	atmp.push(aiger_prop);
	if(ic3.solve(tmp,atmp)){
		printf("SAT\n1\n");
		ic3.printStats();
		exit(10);
	}else{
		printf("UNSAT\n");
		ic3.printStats();
		exit(20);
	}

	return 0;
}
