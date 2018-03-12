/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "cirDef.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void CirMgr::sweep(){
	// need to take care
	vector<unsigned>::iterator it = unused_list.begin();
	while( it != unused_list.end()){
		if(_gatelist[*it]->isAig())
			unused_list.erase(it);
		else
			it++;
	}

	CirGate::setGlobalVisit();
	for(unsigned i=0;i<PO_num;++i){
		VisitDFS(PO_list[i]);
	}
	CirGate* _gate;
	for(unsigned i=1;i <= max_num; ++i){
		_gate = getGate(i);
		if(_gate != NULL){
			if(!_gate->isVisit()){
				SwpDelete(_gate);
			}
		}
	}
	std::sort(unused_list.begin(),unused_list.end());	
	// unused_list.clear();
	//  for(unsigned i = 1; i <= max_num; ++i){
	//     if(getGate(i) != 0){
	//       	if( !_gatelist[i]->havefanout() && _gatelist[i]->getTypeStr() != "UNDEF")
	//         unused_list.push_back((unsigned)i);
	//     }   
	// }
}
void CirMgr::VisitDFS(CirGate* g){
	if(g->isVisit())  
		return;
    g->setVisit();
  	if(g->getFaninA() != NULL )
    	VisitDFS(g->getFaninA());
  	if(g->getFaninB() != NULL )
    	VisitDFS(g->getFaninB());
}

void CirMgr::SwpDelete(CirGate* delgate){
	if( delgate->isAig() || (delgate->getTypeStr() == "UNDEF")){
	cout<<"Sweeping: "<<delgate->getTypeStr()<<"("<<delgate->getID();
	cout<<") removed...\n";
	// delete floating  gate
	if(delgate->getTypeStr() == "UNDEF"){
		for(size_t i = 0, j=delgate->getVector().size(); i<j; ++i){
			unsigned gid = delgate->getFanout(i)->gate()->getID();
			delFloating_in(gid);
		}
	}
	//	delete from fanin

	if ( delgate->getFaninA() != NULL)	{
		 delgate->getFaninA()->delFanout(delgate);
		 if(!delgate->getFaninA()->havefanout() && (delgate->getFaninA()->getTypeStr() != "CONST"))   
		 	unused_list.push_back(delgate->getFaninA()->getID());
	}
	
	if ( delgate->getFaninB() != NULL)  {
		 delgate->getFaninB()->delFanout(delgate);
		 if(!delgate->getFaninB()->havefanout() && (delgate->getFaninB()->getTypeStr() != "CONST"))
		 	unused_list.push_back(delgate->getFaninB()->getID());
	}
	//	delete from fanout
	CirGate* _gate;
	for(size_t i =0, j=delgate->getVector().size(); i<j ;++i){
		_gate = delgate->getFanout(i)->gate();
		if(_gate->getFaninA() == delgate)
			_gate->setFaninA(0,true);
		else
			_gate->setFaninB(0,true);
	}
	// 	delete gate
	if(delgate->isAig()) AIG_num -= 1;
	unsigned gid = delgate->getID();
	delete delgate;
	_gatelist[gid] = 0;
	}
}
// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize(){
	if(DFS_list.empty()){
  		CirGate::setGlobalVisit();
		for(unsigned i=0; i<PO_num; ++i){
	    	cirDFS(PO_list[i]);
		}
  	}
	for(size_t i = 0, j = DFS_list.size(); i<j ; ++i){
		optGates(_gatelist[ DFS_list[i]]);
	}
	RebuildCircuit();
}

void CirMgr::optGates(CirGate* g){
	if((g->getTypeStr() != "AIG") && (g->getTypeStr() != "UNDEF"))
		return;
	//case 3.4 input contains same signal
	if (g->getFaninA()->getID() == g->getFaninB()->getID()){
		if(g->isAinv() == g->isBinv()){
			reWiring(g->getFaninA(), g, g->isAinv());
		}
		else{		// case4
			// if(g->getFaninA()->getVector().size()-2 == 0 && g->getFaninA()->getTypeStr() != "CONST")
			// 	unused_list.push_back(g->getFaninA()->getID());
			cout<<"Simplifying: 0 merging "<<g->getID()<<"...\n";
			CirGate* _gate;
			g->getFaninA()->delFanout(g);
			g->getFaninB()->delFanout(g);
			for(size_t i =0, j= g->getVector().size(); i<j; ++i){	
				_gate = g->getFanout(i)->gate();	
				CirGateV* _gatev = new CirGateV (_gate, 0);
				_gatelist[0]->addFanout(_gatev);		
				if(_gate->getFaninA() == g)
					_gate->setFaninA(_gatelist[0], _gate->isAinv());
				else
					_gate->setFaninB(_gatelist[0], _gate->isBinv());
			}
			if(g->getTypeStr() == "AIG") AIG_num -= 1;
			unsigned gid = g->getID();
			delete g;
			_gatelist[gid] = 0;
		}
	}

	//case 1.2 input contains const 0/1
	else if(g->getFaninA()->getTypeStr() == "CONST"){
		if(g->isAinv())		//case 1	const 1
			reWiring(g->getFaninB(), g, g->isBinv());		
		else{				//case 2 	const 0
			// if(g->getFaninB() != NULL){
			// 	if(g->getFaninB()->getVector().size()-1 ==0 )
			// 		unused_list.push_back(g->getFaninB()->getID());
			// }
			reWiring(g->getFaninA(), g, false);
		}
	}
	else if(g->getFaninB() != NULL){
		if(g->getFaninB()->getTypeStr() == "CONST"){	 
			if(g->isBinv())		//case 1	const 1
				reWiring(g->getFaninA(), g, g->isAinv());
			else{				//case 2 	const 0
				// if(g->getFaninA()->getVector().size()-1 == 0 )
				// 	unused_list.push_back(g->getFaninA()->getID());
				reWiring(g->getFaninB(), g, false);
			}
		}
	}
}

// skip delgate connect its fanin & fnaout
void CirMgr::reWiring(CirGate* input, CirGate* delgate, bool b){
	CirGate* del_out;
	delgate->getFaninA()->delFanout(delgate);
	delgate->getFaninB()->delFanout(delgate);
	bool _b ;
	for(size_t i = 0, j= delgate->getVector().size(); i<j; ++i){
		del_out = delgate->getFanout(i)->gate();
		if(del_out->getFaninA() == delgate){
			_b = xorGate(b, del_out->isAinv());
			del_out->setFaninA(input,_b);
			CirGateV* newgate = new CirGateV(del_out,_b);
			input->addFanout(newgate);
		}
		else{
			_b = xorGate(b, del_out->isBinv());
			del_out->setFaninB(input,_b);
			CirGateV* newgate = new CirGateV(del_out,_b);
			input->addFanout(newgate);
		}
	}
	if(delgate->getTypeStr() == "AIG") AIG_num -= 1;

	unsigned gid = delgate->getID();
	delete delgate;
	_gatelist[gid] = 0;
	cout<<"Simplifying: "<<input->getID()<<" merging ";
	if(b) cout<<"!";
	cout<<delgate->getID()<<"...\n";
}
bool CirMgr::xorGate(bool a, bool b) const {
	if(a)	return !b;
	else	return b;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
