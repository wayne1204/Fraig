/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <stdarg.h>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;
unsigned CirGate::_globalVisit = 0;
// comparing myobject;
// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
string CirGate::getTypeStr() const { 
  	switch (_gatetype){
  		case 0: 
  			return "UNDEF";
  			break;
  		case 1:
  			return "PI";
  			break;
  		case 2:
  			return "PO";
  			break;
  		case 3:
  			return "AIG";
  			break;
  		case 4:
  			return "CONST";
  			break;
  		default:
  			break;
    }
    return "";
}
void CirGate::printSimValue() const {
  for(int i=1; i <= 64 ;++i){
    cout << (value >> (64-i)) % 2;
    if(i % 8 == 0 && i != 64)  cout<<"_";
  }
  cout<<endl;
}

/******************/
/*   print gate   */
/******************/
void PIGate::printGate() const {
  cout<<"PI  "<<getID();
  if(_gatename != "") cout<<" ("<<_gatename<<")";
  cout<<endl;
}

void POGate::printGate() const {
  cout<<"PO  "<<getID()<<" ";
  if(getFaninA()->getTypeStr() == "UNDEF") cout<<"*";
  if(_is_a_neg) cout<<"!";
  cout<<getFaninA()->getID();
  if(_gatename != "") cout<<" ("<<_gatename<<")";
  cout<<endl;
}

void AIGGate::printGate() const {
  cout<<"AIG "<<getID()<<" ";
  if(getFaninA()->getTypeStr() == "UNDEF") cout<<"*";
  if(_is_a_neg) cout<<"!";
  cout<<getFaninA()->getID()<<" ";
  if(getFaninB()->getTypeStr() == "UNDEF") cout<<"*";
  if(_is_b_neg) cout<<"!"; 
  cout<<getFaninB()->getID();
  cout<<endl;
}

void ConstGate::printGate() const {
  cout<<"CONST0"<<endl;
}

/******************/
/*  report gate   */
/******************/

void PIGate::reportGate() const {
  cout << string(80, '=') << endl;
  cout<<"= "<<getTypeStr()<<"("<<getID()<<")";
  if(_gatename != "") cout<<"\""<<_gatename<<"\"";
  cout<<", line "<<getLineNo()<<endl;
  cout<<"= FECs:"<<endl;
  cout<<"= Value: ";
  printSimValue();
  cout << string(80, '=') << endl;
}

void POGate::reportGate() const {
  cout << string(80, '=') << endl;
  cout<<"= "<<getTypeStr()<<"("<<getID()<<")";
  if(_gatename != "") cout<<"\""<<_gatename<<"\"";
  cout<<", line "<<getLineNo()<<endl;
  cout<<"= FECs:"<<endl;
  cout<<"= Value: ";
  printSimValue();
  cout << string(80, '=') << endl;
}

void AIGGate::reportGate() const {
  cout << string(80, '=') << endl;
  cout<<"= "<<getTypeStr()<<"("<<getID()<<")";
  cout<<", line "<<getLineNo()<<endl;
  cout<<"= FECs:";
  cirMgr->printGateFecs(getID(), getGroupNo());
  cout<<"= Value: ";
  printSimValue();
  cout << string(80, '=') << endl;
}

void ConstGate::reportGate() const {
  cout << string(80, '=') << endl;
  cout<<"= "<<getTypeStr()<<"("<<getID()<<")";
  cout<<", line "<<getLineNo()<<endl;
  cout<<"= FECs:";
  cirMgr->printGateFecs(getID(), getGroupNo());
  cout<<"= Value: ";
  printSimValue();
  cout << string(80, '=') << endl;
}
//****************************************************

void CirGate::reportFanin(int level) const {
    assert (level >= 0);
    CirGate::setGlobalVisit();
    printFanin(level, 2);
}

/******************/
/*  print fanin   */
/******************/
void PIGate::printFanin(int level, size_t indent) const {
  cout<<getTypeStr()<<" "<<getID()<<endl;
}

void POGate::printFanin(int level, size_t indent) const {
  cout<<getTypeStr()<<" "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
    if(fan_in_a != 0 && level > 0){   
     setVisit();
       cout<<string(indent, ' ');
       if(_is_a_neg) cout<<"!";
       getFaninA()->printFanin(level -1, indent+2);
    }
}
void AIGGate::printFanin(int level, size_t indent) const {
  cout<<getTypeStr()<<" "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
    if(fan_in_a != 0 && level > 0){   
      setVisit();
       cout<<string(indent, ' ');
       if(_is_a_neg) cout<<"!";
       getFaninA()->printFanin(level -1, indent+2);
    }
    if(fan_in_b != 0 && level > 0){
       setVisit();
       cout<<string(indent, ' ');
       if(_is_b_neg) cout<<"!";
       getFaninB()->printFanin(level -1, indent+2);
    } 
}

void ConstGate::printFanin(int level, size_t indent) const {
  cout<<getTypeStr()<<" "<<getID()<<endl;
}

void UndefGate::printFanin(int level, size_t indent) const {
  cout<<"UNDEF "<<getID()<<endl;
}
//*********************************************
void CirGate::reportFanout(int level) const{
    assert (level >= 0);
    CirGate::setGlobalVisit();
    printFanout(level, 2);
}

/*******************/
/*  print fanout   */
/*******************/

void PIGate::printFanout(int level, size_t indent) const{
  cout<<getTypeStr()<<" "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
  if(level > 0 ){
    for(size_t i=0; i<fan_out.size(); ++i){
      setVisit();
      cout<<string(indent, ' ');
      if(fan_out[i]->isInv()) cout<<"!";
      fan_out[i]->gate()->printFanout(level -1, indent+2);
    }
  }
}
void POGate::printFanout(int level, size_t indent) const{
  cout<<getTypeStr()<<" "<<getID()<<endl;
}

void AIGGate::printFanout(int level, size_t indent) const{
  cout<<getTypeStr()<<" "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
  if(level > 0 ){
    for(size_t i=0; i<fan_out.size(); ++i){
      setVisit();
      cout<<string(indent, ' ');
      if(fan_out[i]->isInv()) cout<<"!";
      fan_out[i]->gate()->printFanout(level -1, indent+2);
    }
  }
}
void ConstGate::printFanout(int level, size_t indent) const{
  cout<<getTypeStr()<<" "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
  if(level > 0 ){
    for(size_t i=0; i<fan_out.size(); ++i){
      setVisit();
      cout<<string(indent, ' ');
      if(fan_out[i]->isInv()) cout<<"!";
      fan_out[i]->gate()->printFanout(level -1, indent+2);
    }
  }
}

void UndefGate::printFanout(int level, size_t indent) const{
  cout<<"UNDEF "<<getID();
  if(isVisit() && level > 0) {
    cout<<" (*)\n";
    return;
  }
  cout<<endl;
  if(level > 0 ){
    for(size_t i=0; i<fan_out.size(); ++i){
      setVisit();
      cout<<string(indent, ' ');
      if(fan_out[i]->isInv()) cout<<"!";
      fan_out[i]->gate()->printFanout(level -1, indent+2);
    }
  }
}
/******************/
/*   write gate   */
/******************/
void PIGate::writeGate(stringstream& outfile) const {
  outfile<<getID()*2<<endl;
}

void POGate::writeGate(stringstream& outfile) const {
  if(isVisit()) return;
    if(_is_a_neg) outfile<<getFaninA()->getID()*2+1<<endl;
    else outfile<<getFaninA()->getID()*2<<endl;
}

void AIGGate::writeGate(stringstream& outfile) const {
  outfile<<getID()*2<<" ";
  if(_is_a_neg) outfile<<getFaninA()->getID()*2+1<<" ";
  else outfile<<getFaninA()->getID()*2<<" ";
  if(_is_b_neg) outfile<<getFaninB()->getID()*2+1<<endl;
  else outfile<<getFaninB()->getID()*2<<endl;
}
