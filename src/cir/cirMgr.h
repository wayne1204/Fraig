/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "cirGate.h"
#include "cirDef.h"
#include "sat.h"

using namespace std;

extern CirMgr *cirMgr;

class CirMgr{
friend class CirGate;
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
    if(gid > max_num+PO_num) return 0;
    if(_gatelist[gid] == 0 ) return 0;
    else return _gatelist[gid];
  }
   // Member functions about circuit construction
   bool readCircuit(const string&);
   bool cutline(string&, unsigned&);
   bool checkWspace(string&);
   bool initGates();
   bool setLink(CirGate*, unsigned&);
   bool RegisterGid(unsigned&);
   bool checkGid(unsigned&, bool&);

   // Member functions about circuit optimization
   void sweep();
   void VisitDFS(CirGate* g);
   void SwpDelete(CirGate* g);

   void optimize();
   void optGates(CirGate* g);
   void reWiring(CirGate* input, CirGate* delgate, bool b);
   bool xorGate(bool a,bool b) const;
   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void simTraversal();
   void initFEC();
   void checkFEC();
   bool checkPattern(string& line);

   // Member functions about fraig
   void strash();
   void MergeGate(CirGate* delgate, CirGate* merging_gate, bool phase);
   void printFEC() const;
   void fraig();
   void initSolver(SatSolver& solver);
   void addClause(CirGate* g, SatSolver& s);
   void SolveConst0(unsigned& _gid, SatSolver& s);
   void reportResult(const SatSolver& solver, bool result);

   // Member functions about circuit reporting
   void printSummary() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printNetlist() const;
   void cirDFS(CirGate*)const;

   void printFECPairs() const;
   void printGateFecs(unsigned _gid, int _groupid) const;
   void writeAag(ostream&) const;
   void writeDFS(stringstream&, CirGate* )const;
   void writeGate(ostream&, CirGate*) const;
   void delFloating_in(unsigned& gid);
   void delFloating_unused(unsigned& gid);
private:
   void RebuildCircuit();
  //for constructing gates
  IdList PA_list;
  IdList AIG_list;

  // for storing CirGate
  CirGate** _gatelist;
  GateList PI_list;
  GateList PO_list;
  mutable IdList DFS_list;
  IdList floating_list;
  IdList unused_list;

  // for simulation
  HashSet<HashNode>         _gateHash;
  vector<vector<FECNode> >   _fecGrps;
  mutable vector<vector<FECNode> >   sorted_fecGrps;
  ofstream           *_simLog;
  // Var*  sat_var;

  //basic infomation
  unsigned max_num;
  unsigned PI_num;
  unsigned FF_num;
  unsigned PO_num;
  unsigned AIG_num; 

};
struct GateComp{
  bool operator()(CirGate* a, CirGate* b){
    return (a->getID() < b->getID());
  }
};
struct FanoutComp{
  bool operator()(CirGateV* a, CirGateV* b){
    return (a->gate()->getID() < b->gate()->getID());
  }
};
struct VectorComp {
  bool operator()(vector<FECNode> a, vector<FECNode> b) const{
    return(a[0].gate()->getID() < b[0].gate()->getID());
  }
};
#endif // CIR_MGR_H
