/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashSet.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
MyUsage uu;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
// static unsigned merging_id = 0;
// static unsigned delgate_id = 0;
static unsigned solve_time = 0;
static vector<Var> sat_var ;
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void CirMgr::strash(){
	_gateHash.init(AIG_num+1);
	if(DFS_list.empty()){
  		CirGate::setGlobalVisit();
		for(unsigned i=0; i<PO_num; ++i)
	    	cirDFS(PO_list[i]);
  	}
	for(size_t i = 0, j = DFS_list.size(); i<j ; ++i){
		HashNode _node = HashNode(_gatelist[ DFS_list[i]]);
		if(_gateHash.check(_node)){
			_gateHash.query(_node);
			if(_gatelist[ DFS_list[i]]->isAig() && _node.gate()->isAig()) {
				cout<<"Strashing: "<<_node.gate()->getID()<<" merging "<<DFS_list[i]<<"..."<<endl;
				MergeGate(_gatelist[ DFS_list[i]], _node.gate(), false);
			}
		}
		else
			_gateHash.insert(_node);
	}
	RebuildCircuit();
	// DFS_list.clear();
	// CirGate::setGlobalVisit();
	// for(unsigned i=0; i<PO_num; ++i)
	//     cirDFS(PO_list[i]);
	// std::sort(unused_list.begin(),unused_list.end());
}

// phase is true when merging IFEC pairs
void CirMgr::MergeGate(CirGate* delgate, CirGate* merging_gate, bool phase){
	// if(delgate->getTypeStr() != "AIG" || merging_gate->getTypeStr() != "AIG") return;
	// delete from fanin
	delgate->getFaninA()->delFanout(delgate);
	delgate->getFaninB()->delFanout(delgate);
	// delete from fanout
	for(size_t i =0, j= delgate->getVector().size(); i<j; ++i){
		// del_out = delgate->getFanout(i);
		CirGateV* del_out = new CirGateV(delgate->getFanout(i)->gate(), (delgate->getFanout(i)->isInv() ^ phase));
		if(del_out->gate()->getFaninA() == delgate){
			del_out->gate()->setFaninA(merging_gate, (del_out->gate()->isAinv() ^ phase) );
			merging_gate->addFanout(del_out);
		}
		else{
			del_out->gate()->setFaninB(merging_gate, (del_out->gate()->isBinv() ^ phase) );
			merging_gate->addFanout(del_out);
		}
		// delete del_out;     //DO NOT DELETE OR U WILL CRASH
		// del_out = NULL;
	}
	unsigned gid = delgate->getID();
	if(!delgate->havefanout())		
		delFloating_unused(gid);
	if((delgate->getFaninA()->getTypeStr() == "UNDEF") || delgate->getFaninB()->getTypeStr() == "UNDEF")
		delFloating_in(gid);
	
	delete delgate;
	--AIG_num ;
	_gatelist[gid] = NULL;
}

void CirMgr::fraig(){
	int cur_group = 0 ;

	if(DFS_list.empty()){
  		CirGate::setGlobalVisit();
		for(unsigned i=0; i<PO_num; ++i)
	    	cirDFS(PO_list[i]);
  	}
	SatSolver solver;
   	
   	unsigned* merging_candidate = new unsigned [_fecGrps.size()]();
   	merging_candidate[0] = 0;
	
	// varialbe initialize
   	solver.initialize();
   	sat_var.reserve(max_num+1);

	for(unsigned i = 0; i <= max_num; ++i){
		// if( getGate(i) != NULL)
			sat_var[i] = solver.newVar();
	}
	_gatelist[0]->setGroupNo(-1);

	for(size_t i = 0, j = DFS_list.size(); i<j; ++i){		
		// blocking 
		if( getGate(DFS_list[i]) == NULL) continue; // some gate may be deleted
		cur_group = _gatelist[ DFS_list[i]]->getGroupNo();
		if( cur_group == -1) continue;

		// case 1 const 0 merging other
		if(cur_group == 0 && _fecGrps[cur_group][0].getID() == 0){
			_gatelist[ DFS_list[i]]->setGroupNo(-1);
			SolveConst0(DFS_list[i], solver);
		}
		// case 2 choosing candidate
		
		else if(merging_candidate[cur_group] == 0 ){
			merging_candidate[cur_group] = DFS_list[i];
			_gatelist[ DFS_list[i]]->setGroupNo(-1);
			continue;
		}

		// case 3 first gate merging 
		else{
			if( ++solve_time > 10){
				SatSolver solver; 
				initSolver(solver);
			}
			CirGate::setGlobalVisit();
			addClause( _gatelist[ merging_candidate[cur_group]], solver);
			CirGate::setGlobalVisit();
			addClause( _gatelist[ DFS_list[i]], solver);
			Var newV = solver.newVar();
			solver.addXorCNF(newV, sat_var[ merging_candidate[cur_group]], _gatelist[merging_candidate[cur_group]]->isSimInv(), 
				sat_var[ DFS_list[i]], _gatelist[ DFS_list[i]]->isSimInv());
			solver.assumeRelease();  // Clear assumptions
			solver.assumeProperty(sat_var[0], false);
			solver.assumeProperty(newV, true);  

			_gatelist[ DFS_list[i]]->setGroupNo(-1);
			if(!solver.assumpSolve()){
				bool phase = ( _gatelist[ merging_candidate[cur_group]]->isSimInv() ^ _gatelist[ DFS_list[i]]->isSimInv());
				cout<<"Fraig: "<<merging_candidate[cur_group]<<" merging ";
				if(phase) cout<<"!";
				cout<< DFS_list[i]<<"...\n";
				MergeGate(_gatelist[ DFS_list[i]], _gatelist[merging_candidate[cur_group]], phase);
			}
		}
	}
	if(!_fecGrps.empty())
		cout<<"Updating by UNSAT... Total #FEC Group = 0\n";	
	
	sorted_fecGrps.clear();
	_fecGrps.clear();	
	delete[] merging_candidate;
	RebuildCircuit();
	// DFS_list.clear();
	// CirGate::setGlobalVisit();
	// for(unsigned i=0; i<PO_num; ++i)
	//     cirDFS(PO_list[i]);
	// std::sort(unused_list.begin(),unused_list.end());
}


void CirMgr::initSolver(SatSolver& solver){
   	solver.initialize(); 
   	sat_var.clear();
	// sat_var.reserve(max_num+1);
	for(unsigned i = 0; i <= max_num; ++i){
		// if( getGate(i) != NULL)
			sat_var[i] = solver.newVar();
	}
	// _gatelist[0]->setGroupNo(-1);
}


void CirMgr::addClause(CirGate* g, SatSolver& s){
	if(g->isVisit() || !g->isAig() )  
		return;
	g->setVisit();	
	addClause(g->getFaninA(), s);
	addClause(g->getFaninB(), s);

	s.addAigCNF(sat_var[g->getID()], sat_var[g->getFaninA()->getID()], g->isAinv(),
				sat_var[g->getFaninB()->getID()], g->isBinv());
}


void CirMgr::SolveConst0(unsigned& _gid, SatSolver& s){
	CirGate::setGlobalVisit();
	addClause( _gatelist[_gid], s);

	s.assumeRelease();  // Clear assumptions
	s.assumeProperty(sat_var[0], false);
	s.assumeProperty(sat_var[_gid], !_gatelist[_gid]->isSimInv());  

	if(!s.assumpSolve()){
		cout<<"Fraig: 0 merging ";
		if( _gatelist[_gid]->isSimInv()) cout<<"!";
		cout<<_gid<<"...\n";
		MergeGate(_gatelist[_gid], _gatelist[0], _gatelist[_gid]->isSimInv());
	}	
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void CirMgr::reportResult(const SatSolver& solver, bool result)
{
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
   if (result) {
      for (size_t i = 0; i <= max_num+PO_num; ++i)
         cout << solver.getValue(sat_var[i]) << endl;
   }
}

