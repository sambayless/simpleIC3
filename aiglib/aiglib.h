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
//Basic C++ interface to aiger
#ifndef _AIGLIB
#define _AIGLIB

extern "C" {
#include "aiger/aiger.h"
}
#include <list>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cassert>
namespace aiglib{
class Circuit;

struct Literal{

	unsigned x;
	Literal():x(aiger_true){

	}
	explicit Literal(int x):x(x){

	}

	bool operator ==(Literal p) const {
		return x == p.x;
	}
	bool operator !=(Literal p) const {
		return x != p.x;
	}
	bool operator <(Literal p) const {
		return x < p.x;
	}
	unsigned toInt() const{
		return x;
	}
	bool isConst()const{
		return x<2;
	}
	bool isConstTrue()const{
		return x==1;
	}
	bool isConstFalse()const{
		return x==0;
	}
};
const Literal True =  Literal(aiger_true);
const Literal False =  Literal(aiger_false);


inline Literal operator ~(Literal p) {
	return Literal(aiger_not(p.toInt()));
}

inline bool sign(Literal p) {
	return p.x & 1;
}
inline int var(Literal p) {
	return p.x >> 1;
}
enum class Value{
	False,True,Undef
};
class Circuit{
	aiger * aig;
	std::vector<Literal> store;
	std::vector<Literal> tmp;

public:

	Circuit(){
		aig = aiger_init ();
	}
	~Circuit(){
		aiger_reset(aig);
	}
private:
	template<typename... Args>
	void collect(std::vector<Literal> & store, Literal a, Args... args ){
		store.push_back(a);
		collect(store,args...);
	}
	void collect(std::vector<Literal> & store, Literal a){
		store.push_back(a);
	}


	Literal bin_op(std::vector<Literal> & store,Literal (Circuit::*f)(Literal,Literal)){
		int n = store.size();
		while(n>1){
			int p = 0;
			for(int i = 0;i<n;i+=2){
				if(i+1<n){
					store[p++] = (this->**&f)(store[i],store[i+1]);
				}else{
					store[p++] = store[i];
				}
			}
			n=p;
		}
		Literal a;
		if(store.size()){
			a = store[0];
			store.clear();
		}else{
			a = getTrue();
		}
		return a;
	}
public:

	void writeToFile(const char * file){
		if(!aiger_open_and_write_to_file(aig,file )){
			throw std::runtime_error("Failed to write AIG file");
		}
	}

	Literal getTrue()const{
		return True;
	}
	Literal getFalse()const{
		return False;
	}

	Literal And(Literal a)
	{
		return a;
	}

	Literal And(Literal a, Literal b){
		//special case these
		if(a.isConst() || b.isConst()){
			if (a.isConstTrue()){
				return b;
			}else if (a.isConstFalse()){
				return a;
			}else if (b.isConstTrue()){
				return a;
			}else if (b.isConstFalse()){
				return b;
			}
		}

		Literal lhs = newFreeLit();
		aiger_add_and(aig,lhs.toInt(),a.toInt(),b.toInt());
		return lhs;
	}
	Literal And(const std::list<Literal> & vals){

		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::And);

	}
	Literal And(const std::vector<Literal> & vals){

		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::And);
	}

	template<typename... Args>
	Literal And(Literal a, Literal b, Args... args)
	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);

		return And(store);
	}



	Literal Or(Literal a)
	{
		return a;
	}

	Literal Or(Literal a, Literal b){
		return ~And(~a, ~b);
	}

	Literal Or(const std::list<Literal> & vals){

		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::Or);

	}
	Literal Or(const std::vector<Literal> & vals){
		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::Or);
	}

	template<typename... Args>
	Literal Or(Literal a, Literal b, Args... args)
	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);

		return Or(store);
	}



	Literal Nor(Literal a)
	{
		return ~a;
	}

	Literal Nor(Literal a, Literal b){
		return ~Or(a,b);
	}

	Literal Nor(const std::list<Literal> & vals){
		return ~Or(vals);
	}
	Literal Nor(const std::vector<Literal> & vals){
		return ~Or(vals);
	}

	template<typename... Args>
	Literal Nor(Literal a, Literal b, Args... args)	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);

		return Nor(store);
	}


	Literal Nand(Literal a)
	{
		return ~a;
	}

	Literal Nand(Literal a, Literal b){
		return ~And(a,b);
	}
	Literal Nand(const std::list<Literal> & vals){
		return ~And(vals);

	}
	Literal Nand(const std::vector<Literal> & vals){
		return ~And(vals);
	}

	template<typename... Args>
	Literal Nand(Literal a, Literal b, Args... args)
	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);
		return Nand(store);
	}

	Literal Xor(Literal a)
	{
		return ~a;
	}

	Literal Xor(Literal a, Literal b){
		if(a.isConst() || b.isConst()){
			if (a.isConstTrue()){
				return ~b;
			}else if (a.isConstFalse()){
				return b;
			}else if (b.isConstTrue()){
				return ~a;
			}else if (b.isConstFalse()){
				return a;
			}
		}
		return Or(And(a, ~b), And(~a,b));
	}

	Literal Xor(const std::list<Literal> & vals){
		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::Xor);

	}
	Literal Xor(const std::vector<Literal> & vals){
		tmp.clear();
		for (Literal l:vals)
			tmp.push_back(l);
		return bin_op(tmp,&Circuit::Xor);
	}

	template<typename... Args>
	Literal Xor(Literal a, Literal b, Args... args)
	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);
		return Xor(store);
	}


	Literal Xnor(Literal a)
	{
		return a;
	}
	Literal Xnor(Literal a, Literal b){
		return ~Xor(a,b);
	}
	Literal Xnor(const std::list<Literal> & vals){
		return ~Xor(vals);

	}
	Literal Xnor(const std::vector<Literal> & vals){
		return ~Xor(vals);
	}

	template<typename... Args>
	Literal Xnor(Literal a, Literal b, Args... args)
	{
		store.clear();
		store.push_back(a);
		collect(store,b,args...);
		return Xnor(store);
	}




	Literal Ite(Literal cond, Literal thn, Literal els=False){
		Literal l = ~And(cond,~thn);
		Literal r = ~And(~cond,~els);
		return And(l,r);
	}

	Literal newInput(const char * symbol=""){
		Literal in = newFreeLit();
		aiger_add_input(aig,in.toInt(),symbol);
		return in;
	}

	Literal newLatch(Literal next, Value initialValue = Value::False,const char * symbol=""){
		Literal previous = newFreeLit();
		makeLatch(previous,next,initialValue,symbol);
		return previous;
	}


	//Use carefully!
	Literal newFreeLit(){
		aig->maxvar++;
		return Literal(2*aig->maxvar);
	}

	//Use carefully!
	int makeLatch(Literal previous,Literal next, Value initialValue = Value::False,const char * symbol=""){
		aiger_add_latch(aig,previous.toInt(),next.toInt(),symbol);
		setLatchReset(previous,initialValue);
		return aig->num_latches-1;
	}

	void setLatchReset(Literal latch, Value initialValue = Value::False){
		if(initialValue==Value::False){
			aiger_add_reset(aig,latch.toInt(), aiger_false);
		}else if(initialValue==Value::True){
			aiger_add_reset(aig,latch.toInt(), aiger_true);
		}else{
			aiger_add_reset(aig,latch.toInt(),latch.toInt());
		}
	}

	Value getLatchReset(int latchNumber){
		int reset = aig->latches[latchNumber].reset;
		if(reset==aiger_false){
			return Value::False;
		}else if(reset==aiger_true){
			return Value::True;
		}else{
			return Value::Undef;
		}
	}

	void newConstraint(Literal p,const char * symbol=""){
		aiger_add_constraint(aig,p.toInt(),symbol);
	}

	int newOutput(Literal from, const char * symbol=""){
		aiger_add_output(aig,from.toInt(),symbol);
		return aig->num_outputs-1;
	}

	int newBadState(Literal from, const char * symbol=""){
		aiger_add_bad(aig,from.toInt(),symbol);
		return aig->num_bad-1;
	}

	Literal getLatch(int n, bool next=false){
		assert(n<aig->num_latches);
		if(!next)
			return Literal(aig->latches[n].lit);
		else
			return Literal(aig->latches[n].next);
	}



	int numInputs()const{
		return aig->num_inputs;
	}
	int numLatches()const{
		return aig->num_latches;
	}
	int numAnds()const{
		return aig->num_ands;
	}
	int numOutputs()const{
		return aig->num_outputs;
	}
	int numBadStates()const{
		return aig->num_bad;
	}
	int numConstraints()const{
		return aig->num_constraints;
	}
	Literal getOutput(int n){
		assert(n<aig->num_outputs);
		return Literal(aig->outputs[n].lit);
	}
	Literal getConstraint(int n){
		assert(n<aig->num_constraints);
		return Literal(aig->constraints[n].lit);
	}
	Literal getInput(int n){
		assert(n<aig->num_inputs);
		return Literal(aig->inputs[n].lit);
	}
	Literal getAndGate(int n){
		assert(n<aig->num_ands);
		return Literal(aig->ands[n].lhs);
	}
	Literal getAndGateInput(int n, bool get_rhs1=false){
		assert(n<aig->num_ands);
		if(get_rhs1){
			return Literal(aig->ands[n].rhs1);
		}else{
			return Literal(aig->ands[n].rhs0);
		}
	}

	void reencode(){
		aiger_reencode(aig);
	}

	//Access C aiger library
	aiger * getAIG(){
		return aig;
	}
};
};
#endif
