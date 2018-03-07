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

#ifndef WORD_H_
#define WORD_H_
#include "aiglib.h"
#include <vector>
using namespace aiglib;

#include <cassert>
#include <stdexcept>
class Word{
	Circuit & circuit;
	std::vector<Literal> data;
public:
	friend Word Ite(Literal cond, Word   thn, Word   els);
	Word(Circuit & circuit,std::vector<Literal>&vals):circuit(circuit){
		for(Literal l:vals)
			data.push_back(l);
	}
	Word(Circuit & circuit):circuit(circuit){

	}

	Word(const Word & from):circuit(from.circuit),data(from.data){

	}
	Word(Word & from):circuit(from.circuit),data(from.data){

	}
	Word(Word && from):circuit(from.circuit),data(std::move(from.data)){

	}
	const Literal* begin() const {
		return &data[0];
	}

	const Literal* end() const {
		return &data[data.size()];
	}
/*
	Word& operator=(Word&& other) {
        return *this;
    }*/
	Word& operator=(const Word& other) {
		 if(&other == this)
			 return *this;
		 data.clear();
		 for(Literal l:other.data)
			 data.push_back(l);
		 return *this;
	 }
	static Word makeInputWord(Circuit & circuit, int wordSize){
		Word w(circuit);
		for(int i = 0;i<wordSize;i++)
			w.data.push_back(circuit.newInput());
		return w;
	}
	static Word makeFreeWord(Circuit & circuit, int wordSize){
		Word w(circuit);
		for(int i = 0;i<wordSize;i++){
			w.data.push_back(circuit.newFreeLit());
		}

		return w;
	}
	static Word makeConstant(Circuit & circuit, int wordSize, long const_val){
		Word w(circuit);
		for(int i = 0;i<wordSize;i++){
			if (const_val& (1L<<i)){
				w.data.push_back(True);
			}else{
				w.data.push_back(False);
			}
		}
		return w;
	}

	int size() const{
		return data.size();
	}

	bool isConst() const{
		for(Literal l:data)
			if(!l.isConst())
				return false;

		return true;
	}

	long long getConstantValue()const{
		if(!isConst())
			throw std::runtime_error("Word could not be statically proven constant!");
		if(size()>64)
			throw std::runtime_error("Wordsize is too large to convert to 64 bit long");
		long long value=0;
		for(int i = 0;i<size();i++){
			if(data[i].isConstTrue())
				value+=1L<<i;
		}
		return value;
	}

/*
	Word operator[] (const int i){
		Word w(circuit);
		w.data.push_back(data[i]);
		return w;
	}*/

	Literal operator[](const int i)const{
		return data[i];
	}

	Word operator ~()const{
		return invert();
	}

	Word invert()const{
		Word w(circuit);
		for (Literal l:data)
			w.data.push_back(~l);
		return w;
	}

	Word operator ^(const Word a)const{
		assert(a.size()==size());
		Word w(circuit);
		for (int i = 0;i<data.size();i++){
			w.data.push_back(circuit.Xor(data[i], a.data[i]));
		}
		return w;
	}

	Word operator |(const Word a)const{
		assert(a.size()==size());
		Word w(circuit);
		for (int i = 0;i<data.size();i++){
			w.data.push_back(circuit.Or(data[i], a.data[i]));
		}
		return w;
	}

	Word operator &(const Word a)const{
		assert(a.size()==size());
		Word w(circuit);
		for (int i = 0;i<data.size();i++){
			w.data.push_back(circuit.And(data[i], a.data[i]));
		}
		return w;
	}

	Word slice(int lower, int upper)const{
		Word w(circuit);
		assert(lower>=0);
		assert(upper<=size());
		for(int i = lower;i<upper;i++){
			w.data.push_back(data[i]);
		}
		return w;
	}

	Word resize(int newsize, Literal defaultVal = False){
		Word w(circuit);
		if(newsize>=size()){
			for(int i = 0;i<size();i++){
				w.data.push_back(data[i]);
			}
			for(int i = size();i<newsize;i++){
				w.data.push_back(defaultVal);
			}
		}else{
			for(int i = 0;i<newsize;i++){
				w.data.push_back(data[i]);
			}
		}
		return w;
	}

	Word lshift(int arg)const{
		assert(arg>=0);
		if(arg==0)
			return *this;
		if(arg>size()){
			arg=size();
		}

		Word w(circuit);

		for(int i = 0;i<arg;i++){
			w.data.push_back(False);
		}

		for (int i = arg;i<data.size();i++){
			w.data.push_back(data[i-arg]);
		}
		return w;
	}
	Word rshift(int arg)const{
		assert(arg>=0);
		if(arg==0)
			return *this;
		if(arg>size()){
			arg=size();
		}

		Word w(circuit);

		for(int i = 0;i<data.size()-arg;i++){
			w.data.push_back(data[i+arg]);
		}
		Literal last = data.back();
		for (int i = 0;i<arg;i++){
			w.data.push_back(last);
		}
		return w;
	}

	Word logical_rshift(int arg)const{
		assert(arg>=0);
		if(arg==0)
			return *this;
		if(arg>size()){
			arg=size();
		}

		Word w(circuit);
		for(int i = 0;i<data.size()-arg;i++){
			w.data.push_back(data[i+arg]);
		}
		for (int i = 0;i<arg;i++){
			w.data.push_back(False);
		}
		return w;
	}

	//Returns this word, interpreted as a (signed) integer, plus 1
	Word inc(){
		Word w(circuit);
		Word & a = *this;
		Literal carry;
		Literal sum = halfAdder(a[0],True,carry);
		w.data.push_back(sum);
		for(int i = 1;i<size();i++){
			Literal x1 = a[i];
			w.data.push_back(circuit.Xor(x1, carry));
			carry = circuit.And(x1,carry);
		}
		return w;
	}
	Word dec(){
		Word w(circuit);
		Word & a = *this;
		Literal carry;
		Literal sum = halfAdder(a[0],True,carry);
		w.data.push_back(sum);
		for(int i = 1;i<size();i++){
			Literal x1 = circuit.Xor(a[i],True);
			w.data.push_back(circuit.Xor(x1, carry));
			Literal c1 = circuit.And(x1,carry);
			carry = circuit.Or(circuit.And(a[i],True),c1);
		}
		return w;
	}
	Word zero(){
		Word w(circuit);
		for(int i = 0;i<size();i++)
			w.data.push_back(False);
		return w;
	}
	Word one(){
		Word w(circuit);
		w.data.push_back(True);
		for(int i = 1;i<size();i++)
			w.data.push_back(False);
		return w;
	}
	Word ones(){
		Word w(circuit);
		for(int i = 0;i<size();i++)
			w.data.push_back(True);
		return w;
	}

	Word add(Word  b){
		Literal ignore;
		return add(b,ignore);
	}
	//Simple adder
	Word add(Word   b, Literal & carry_out){
		assert(b.size()==size());
		Word w(circuit);
		Word & a = *this;
		Literal carry;
		Literal sum = halfAdder(a[0],b[0],carry);
		w.data.push_back(sum);
		for(int i = 1;i<size();i++){
			Literal x1 = circuit.Xor(a[i],b[i]);
			w.data.push_back(circuit.Xor(x1, carry));
			Literal c1 = circuit.And(x1,carry);
			carry = circuit.Or(circuit.And(a[i],b[i]),c1);
		}
		carry_out= carry;
		return w;
	}

	//Two's complement negation
	Word negate(){
		return invert().inc();
	}

	Literal isEqual(Word w){
		assert(w.size()==size());
		Literal r = True;
		for(int i = 0;i<w.size();i++){
			r = circuit.And(r,circuit.Xnor(w[i],data[i]));
		}
		return r;
	}

	//True if (signed, two's complement) value is >0
	Literal isPositive(){
		return circuit.And(~isNegative(), ~isZero());
	}

	Literal isZero(){
		return circuit.Nor(data);
	}

	Literal isNegative(){
		return data.back();//negative if the sign bit is on
	}

	//Demux this word into a unary representation (creating a word with 2^wordSize lits, so this is expensive!)
	Word demux(){

		long max_val = 1L<< size();
		if(isConst()){
			Word w(circuit);
			long v = getConstantValue();
			for(int i = 0;i<max_val;i++){
				w.data.push_back(i==v ? True:False);
			}
			return w;
		}

		if(size()<=1){
			return *this;
		}
		if(size()==2){
			return demux_2to4(data[0],data[1]);
		}
		if(size()==3){
			return demux_3to8(data[0],data[1],data[2]);
		}
		if(size()==4){
			return demux_4to16(data[0],data[1],data[2],data[3]);
		}
		if(size()>4 && size()<8){
			std::vector<Literal> tmp;
			for(int i = 0;i<size();i++){
				tmp.push_back(data[i]);
			}
			for(int i= size();i<8;i++){
				tmp.push_back(circuit.getFalse());
			}
			return demux_8to256(data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
		}
		if(size()==8){
			return demux_8to256(data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
		}
		throw  std::runtime_error("Not implemented");

	}

private:

	 Word demux_2to4(Literal a, Literal b){
		Word w(circuit);
		w.data.push_back(circuit.And(~a, ~b));//0
		w.data.push_back(circuit.And(a, ~b));//1
		w.data.push_back(circuit.And(~a, b));//2
		w.data.push_back(circuit.And(a, b));//3
		return w;
	}

	 Word demux_3to8(Literal a, Literal b,Literal c){
		Word w(circuit);
		w.data.push_back(circuit.And(~a, ~b, ~c));
		w.data.push_back(circuit.And(a, ~b, ~c));
		w.data.push_back(circuit.And(~a, b, ~c));
		w.data.push_back(circuit.And(a, b, ~c));
		w.data.push_back(circuit.And(~a, ~b, c));
		w.data.push_back(circuit.And(a, ~b, c));
		w.data.push_back(circuit.And(~a, b, c));
		w.data.push_back(circuit.And(a, b, c));
		return w;
	}

	 Word demux_4to16(Literal a, Literal b,Literal c, Literal d){
		Word w(circuit);
		Word i1 = demux_2to4(a,b);
		Word i2 = demux_2to4(c,d);
		int p = 0;
		for(int i = 0;i<i2.size();i++){
			for(int j = 0;j<i1.size();j++){
				w.data.push_back(circuit.And(i1[j],i2[i]));
			}
		}
		return w;
	}
	 Word demux_8to256(Literal a, Literal b,Literal c, Literal d,Literal e, Literal f,Literal g, Literal h){
		Word w(circuit);
		Word i1 = demux_4to16(a,b,c,d);
		Word i2 = demux_4to16(e,f,g,h);
		for(int i = 0;i<i2.size();i++){
			for(int j = 0;j<i1.size();j++){
				w.data.push_back(circuit.And(i1[j],i2[i]));
			}
		}
		return w;
	}
	Literal halfAdder(Literal a, Literal b, Literal & carry_out){
		carry_out = circuit.And(a,b);
		return circuit.Xor(a,b);
	}

};

inline Word Ite(Literal cond, Word    thn, Word   els){
	assert(thn.size()==els.size());
	Word w(thn.circuit);
	for(int i = 0;i<thn.size();i++){
		w.data.push_back(w.circuit.Ite(cond,thn[i],els[i]));
	}
	return w;
}

#endif /* WORD_H_ */
