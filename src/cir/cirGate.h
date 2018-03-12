/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "myHashSet.h"
using namespace std;

class CirGate;
struct comparing;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGateV {
  #define NEG 0x1
public:  
  CirGateV(CirGate* g, size_t phase):  _gateV(size_t(g) + phase) { }
  CirGate* gate() const {
    return (CirGate*)(_gateV & ~size_t(NEG)); 
  }
  bool isInv() const { return (_gateV & NEG); }
private:
  size_t _gateV;
};

class CirGate
{
public:
  CirGate() {}
  CirGate(unsigned n, unsigned l, int type)
  :_gid(n), _lineNo(l), _gatetype(type){
    _visit = 0;
    _groupNo = -1;
    _simInv = false;
    value = (size_t)0;
  }
  virtual ~CirGate() {}

  // Basic access methods
  unsigned getID() const { return _gid;}
  int getLineNo() const { return _lineNo; }
  string getTypeStr() const;
  bool isAig(){ return (_gatetype == AIG_GATE);}
  virtual string getGateName() const { return ""; }
  virtual void setGateName(string& s) {}

  // simulation funciton
  void  setSimValue(const size_t& v) { value = v;}
  size_t  getSimValue() const { return value; }
  size_t& getSimValue()  { return value; }
  void  printSimValue() const;
  void  setGroupNo(int i) { _groupNo = i; }
  int   getGroupNo() const { return _groupNo; }
  void  setSimInv(bool b) { _simInv  = b; }
  bool  isSimInv() const { return _simInv; }

  // Printing functions
  virtual void printGate() const {}
  virtual void reportGate() const {}
  void reportFanin(int level) const;
  virtual void printFanin(int level, size_t indent) const {}
  void reportFanout(int level) const;
  virtual void printFanout(int level, size_t indent) const {}
  virtual void writeGate(stringstream& outfile) const {}

  //fan_in functions
  virtual CirGate* getFaninA() const =0;
  virtual void setFaninA(CirGate* g, bool b) {}
  virtual bool isAinv() const = 0;
  virtual CirGate* getFaninB() const =0;
  virtual void setFaninB(CirGate* g, bool b) {}
  virtual bool isBinv() const = 0 ;

  //fan_out functions
  virtual bool havefanout() const = 0;
  virtual CirGateV* getFanout(unsigned i) const = 0;
  virtual vector<CirGateV*>& getVector() = 0;
  virtual void addFanout(CirGateV* g) {} 
  virtual void delFanout(const CirGate* g) {}
  //[warning] need to take care delFanout not passing cirgatev
  //visit function
  static void setGlobalVisit() {_globalVisit++; }
  void setVisit() const { _visit = _globalVisit; }
  bool isVisit() const { return _visit == _globalVisit; }
   
protected:
  //gate info
  bool  _simInv;
  unsigned _gid;
  int _lineNo;
  int _groupNo;
  int _gatetype;
  size_t value;

  //for searching
  static  unsigned _globalVisit;
  mutable unsigned _visit;
};

class PIGate : public CirGate
{
public:
  PIGate(unsigned n, unsigned l, int type) 
    : CirGate(n,l,type) {}
  ~PIGate() {}

  //access function
  string getGateName() const { return _gatename;}
  void setGateName(string& s) { _gatename = s; }

  // Printing functions
  void printGate() const; 
  void reportGate() const;
  void printFanin(int level, size_t indent) const;
  void printFanout(int level, size_t indent) const ;
  void writeGate(stringstream& outfile) const;

  //fan_in & fan_out
  CirGate* getFaninA() const { return NULL; }
  bool isAinv() const { return true;} 
  CirGate* getFaninB() const { return NULL; }
  bool isBinv() const { return true;} 

  bool havefanout() const { return !fan_out.empty(); }
  CirGateV* getFanout(unsigned i) const { return fan_out[i]; }
  void addFanout(CirGateV* g) { fan_out.push_back(g); } 
  void delFanout(const CirGate* g) {
    for(vector<CirGateV*>::iterator it = fan_out.begin(); it != fan_out.end(); ++it){
      if((*it)->gate() == g){
        fan_out.erase(it);
        return;
      }
    }
  }
  vector<CirGateV*>& getVector() {
    return fan_out;
  }
private:
  string _gatename;
  vector<CirGateV*> fan_out;
};

class POGate : public CirGate{
public:
  POGate(unsigned n, unsigned l, int type) 
  : CirGate(n,l,type) {
    fan_in_a = NULL;
  }
  ~POGate() {}
  //access fuction
  string getGateName()const { return _gatename;}
  void setGateName(string& s) { _gatename = s; }
  // Printing functions
  void printGate() const; 
  void reportGate() const;
  void printFanin(int level, size_t indent) const;
  void printFanout(int level, size_t indent) const ;
  void writeGate(stringstream& outfile) const;
  
  //fan_in & fan_out
  CirGate* getFaninA()const{ return fan_in_a; }
  void setFaninA(CirGate* g, bool b) { 
    fan_in_a = g;    _is_a_neg = b;
  }
  bool isAinv() const { return _is_a_neg;} 
  CirGate* getFaninB()const{ return NULL; }
  bool isBinv() const { return true;} 

  bool havefanout() const { return false; }
  CirGateV* getFanout(unsigned i) const {return NULL; }
  vector<CirGateV*>& getVector() {
    vector<CirGateV*> a;
    return a;
  }
private:
  string _gatename;
  CirGate* fan_in_a;
  bool _is_a_neg;
};

class AIGGate : public CirGate{
public:
  AIGGate(unsigned n, unsigned l, int type) 
  : CirGate(n,l,type) {
    fan_in_a = NULL;
    fan_in_b = NULL;
  }
  ~AIGGate() {}

  // Printing functions
  void printGate() const; 
  void reportGate() const;
  void printFanin(int level, size_t indent) const;
  void printFanout(int level, size_t indent) const;
  void writeGate(stringstream& outfile) const;

  //fan_in & fan_out
  CirGate* getFaninA() const { return fan_in_a; }
  void setFaninA(CirGate* g, bool b) { 
    fan_in_a = g;    _is_a_neg = b;
  }
  bool isAinv() const { return _is_a_neg;} 
  CirGate* getFaninB() const { return fan_in_b; }
  void setFaninB(CirGate* g, bool b) { 
     fan_in_b = g;    _is_b_neg = b;
  } 
  bool isBinv() const { return _is_b_neg;} 
  bool havefanout() const { return !fan_out.empty(); }
  CirGateV* getFanout(unsigned i) const { return fan_out[i]; }
  void addFanout(CirGateV* g) { fan_out.push_back(g); } 
  void delFanout(const CirGate* g) {
    for(vector<CirGateV*>::iterator it = fan_out.begin(); it != fan_out.end(); ++it){
      if((*it)->gate() == g){
        fan_out.erase(it);
        return;
      }
    }
  }
  vector<CirGateV*>& getVector() {
    return fan_out;
  }
private:
  //links to other gates
  vector<CirGateV*> fan_out;
  CirGate* fan_in_a;
  CirGate* fan_in_b;
  bool _is_a_neg;
  bool _is_b_neg;
};

class ConstGate : public CirGate{
public:
  ConstGate(unsigned n, unsigned l, int type) 
  : CirGate(n,l,type)
  {}
  ~ConstGate() {}
  // Printing functions
  void printGate() const; 
  void reportGate() const;
  void printFanin(int level, size_t indent) const;
  void printFanout(int level, size_t indent) const ;
  void writeGate(stringstream& outfile) const {}
  
  //fan_out
  CirGate* getFaninA() const { return NULL; }
  bool isAinv() const { return true;} 
  CirGate* getFaninB() const { return NULL; }
  bool isBinv() const { return true;} 
  bool havefanout() const { return !fan_out.empty(); }
  CirGateV* getFanout(unsigned i) const { return fan_out[i]; }
  void addFanout(CirGateV* g) {  fan_out.push_back(g); }  
  void delFanout(const CirGate* g) {
    for(vector<CirGateV*>::iterator it = fan_out.begin(); it != fan_out.end(); ++it){
      if((*it)->gate() == g){
        fan_out.erase(it);
        return;
      }
    }
  }
  vector<CirGateV*>& getVector() {
    return fan_out;
  }
private:
  vector<CirGateV*> fan_out;
};

class UndefGate : public CirGate{
public:
  UndefGate(unsigned n, unsigned l, int type) 
  : CirGate(n,l,type) {}
  ~UndefGate() {}

  void printFanin(int level, size_t indent) const;
  void printFanout(int level, size_t indent) const;
  //fan_out
  CirGate* getFaninA() const { return NULL; }
  bool isAinv() const { return true;} 
  CirGate* getFaninB() const { return NULL; }
  bool isBinv() const { return true;} 

  bool havefanout() const { return !fan_out.empty(); }
  CirGateV* getFanout(unsigned i) const { return fan_out[i]; }
  void addFanout(CirGateV* g) { fan_out.push_back(g); } 
  void delFanout(const CirGate* g) {
    for(vector<CirGateV*>::iterator it = fan_out.begin(); it != fan_out.end(); ++it){
      if((*it)->gate() == g){
        fan_out.erase(it);
        return;
      }
    }
  }
  vector<CirGateV*>& getVector() {
    return fan_out;
  }
private:
  vector<CirGateV*> fan_out;
};

class HashNode{
public:
  HashNode();
  HashNode(CirGate* g) : _gate(g) {
    num1=0; num2=0;
    if(g->getFaninA() != NULL)
      num1 = g->getFaninA()->getID()*2 + (unsigned)g->isAinv();
    if(g->getFaninB() != NULL)
      num2 = g->getFaninB()->getID()*2 + (unsigned)g->isBinv();
  }
  ~HashNode() {}
  size_t operator()() const{ //Hash function
    return (size_t)num1 * num2;
  }
  bool operator == (const HashNode& k) const{
    if(num1 == k.num1)
    {    
          return (num2 == k.num2);
    }
    else if(num1 == k.num2)
      return (num2 == k.num1);
    else  
      return false;
   }
  CirGate* gate() const { return _gate; }
private:
  CirGate* _gate;
  unsigned num1;
  unsigned num2;
};

class FECNode{
public:
  FECNode();
  FECNode(CirGate* g, bool neg) :_isNeg(neg){
    _gate = g;
    sim_value = g->getSimValue();
    if(neg) sim_value = ~sim_value;
  }
  CirGate* gate() const { return _gate;}
  unsigned getID() const { return _gate->getID(); }
  size_t operator()() const{ //Hash function
    return sim_value;
  }
  void updateValue( size_t& _v) { 
    sim_value = _v; 
    if(_isNeg) sim_value = ~sim_value;
  }
  bool isInv() const { return _isNeg; }
  void setInv(bool b) { _isNeg = b; }
  bool operator == (const FECNode& k) const{
    // if(isInv() || k.isInv()){
    //   return (sim_value == ~k.sim_value);
    // }
    return (sim_value == k.sim_value) ;
  }

private:
  bool      _isNeg;
  CirGate*   _gate;
  size_t   sim_value;
};

#endif // CIR_GATE_H
