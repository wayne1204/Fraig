/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctype.h>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
size_t fa = 0;
size_t fb = 0;
bool initialize = false;
size_t length = 0;
size_t indent = 0;
GateComp gcomp;
	
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned patternline = 0;
static unsigned pos = 0;

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void CirMgr::randomSim(){
	initialize = false; 
  	patternline = 0;
  	size_t last_size = 0;
  	unsigned same_time = 0;

  	if(DFS_list.empty()){
  		CirGate::setGlobalVisit();
		for(unsigned i = 0; i < PO_num; ++i)
	    	cirDFS(PO_list[i]);
  	}

  	size_t* sim_64 = new size_t [PI_num] ();
  	pos = 64;
  	
  	while(same_time <= 10){
  		for(unsigned i = 0; i < pos ; ++i){
  			for(unsigned j = 0; j < PI_num; ++j){
	  			sim_64[j] =  ((size_t)rnGen(2) << i) | sim_64[j];
	  		}
  		} 		
  		for(unsigned j = 0; j < PI_num; ++j){
  			PI_list[j]->setSimValue(sim_64[j]);
  			sim_64[j] = 0;
  		}
  		simTraversal();
  		patternline += 64;
  		if(_fecGrps.size() == last_size)
  			++ same_time;
  		else{
  			same_time = 0;
  			last_size = _fecGrps.size();
  		}

  	}
  	cout<<"\r"<<patternline<<" patterns simulated.\n";
}

void CirMgr::fileSim(ifstream& patternFile){
  	string line;
	initialize = false;
  	pos = 1; patternline = 0;
  	if(DFS_list.empty()){
  		CirGate::setGlobalVisit();
		for(unsigned i = 0; i < PO_num; ++i)
	    	cirDFS(PO_list[i]);
  	}
  	patternFile >> line;
  	// getline(patternFile,line);
  	if(!checkPattern(line))		return;
  	size_t* sim_64 = new size_t [PI_num];
  	for(unsigned i=0; i<PI_num; ++i)
  		sim_64[i] = (size_t)(line[i+indent]-'0');

  	while(patternFile >> line){
  		if(!checkPattern(line))		return;
  		for(unsigned i=0; i<PI_num; ++i){
	  		sim_64[i] = ((size_t)(line[i+indent]-'0') << pos) | sim_64[i];
  		}
  		if(++pos == 64){	// sim and reset
  			for(unsigned i=0; i<PI_num; ++i){
  				PI_list[i]->setSimValue(sim_64[i]);
  				sim_64[i] = 0;
  			}
  			simTraversal();
  			// ostream
  			pos = 0; patternline += 64;
  		}
  	}
  	if( pos != 0){
  		patternline += pos;
  		for(unsigned i=0; i<PI_num; ++i)
			PI_list[i]->setSimValue(sim_64[i]);
		simTraversal();
		// ostream
  	}	
  	cout<<"\r"<<patternline<<" patterns simulated.\n";
}


//*** recursively update simulation value ***
void CirMgr::simTraversal(){
	for(size_t i = 0, j = DFS_list.size(); i<j ; ++i){
		if(_gatelist[DFS_list[i]]->isAig()){
			fa = _gatelist[ DFS_list[i]]->getFaninA()->getSimValue();
	    	fb = _gatelist[ DFS_list[i]]->getFaninB()->getSimValue();
	    	if( _gatelist[ DFS_list[i]]->isAinv()) fa =  ~fa ;
	    	if( _gatelist[ DFS_list[i]]->isBinv()) fb =  ~fb ;
	    	_gatelist[ DFS_list[i]]->setSimValue( fa & fb );
		}
		else if(_gatelist[ DFS_list[i]]->getTypeStr() == "PO"){
			fa = _gatelist[ DFS_list[i]]->getFaninA()->getSimValue();
	    	if( _gatelist[ DFS_list[i]]->isAinv()) fa = ~fa;
	    	_gatelist[ DFS_list[i]]->setSimValue(fa);
		}
	}
	if( _simLog != NULL){
		ostream& Outfile = *_simLog;
		for(unsigned i = 0; i < pos; ++i){
			for(unsigned j = 0; j < PI_num; ++j){
				size_t c = (PI_list[j]->getSimValue() >> i) % 2;
				Outfile << c;
			}
			_simLog->put(' ');
			for(unsigned j = 0; j < PO_num; ++j){
				size_t c = (PO_list[j]->getSimValue() >> i) % 2;
				Outfile << c;
			}
			_simLog->put('\n');
		}
	}
	
	
  	if(initialize) checkFEC();
  	else initFEC();
}

//*** divide gates in dfs_list into fec pairs ***
void CirMgr::initFEC(){
	initialize = true;
	HashSet<FECNode>    _fecHash;
	_fecHash.init((AIG_num+1)*30);
	FECNode _fnode = FECNode(_gatelist[0], false);
	_fecHash.insert(_fnode);

	vector<unsigned> sorted_Dfslist = DFS_list;
	std::sort(sorted_Dfslist.begin(), sorted_Dfslist.end());
	// sim & inserting to Hash 
	for(size_t i = 0, j = sorted_Dfslist.size(); i < j; ++i){
		if( !_gatelist[sorted_Dfslist[i]]->isAig()) continue;
		FECNode _fnode = FECNode( _gatelist[sorted_Dfslist[i]], false);
		if( _fecHash.check(_fnode))
			_fecHash.forceInsert(_fnode);
		else{
			FECNode _fnode_b = FECNode( _gatelist[sorted_Dfslist[i]], true);
			if( _fecHash.check(_fnode_b)) {
				_fnode_b.gate()->setSimInv(true);
				_fecHash.forceInsert(_fnode_b);
			}
			else
				_fecHash.insert(_fnode);
		} 		
	}
	// collect fec grps
	for(size_t j = 0, num = _fecHash.numBuckets(); j < num; ++j){
		if(_fecHash[j].size() > 1){
			vector<FECNode> _vec;
			for(size_t k =0; k < _fecHash[j].size(); k++){
				_fecHash[j][k].gate()->setGroupNo(_fecGrps.size());
				_vec.push_back(_fecHash[j][k]);
			}
			_fecGrps.push_back(_fecHash[j]);
		}
	}
	cout<<"\rTotal #FEC Group = "<<_fecGrps.size()<< std::flush;
}

//*** divide gates in fec groups into fec subgroups ***
void CirMgr::checkFEC(){
	vector<vector<FECNode> > newGrps;
	for(size_t row = 0, rowsize = _fecGrps.size(); row < rowsize; ++row){
		HashSet<FECNode>    _fecHash;
		_fecHash.init(_fecGrps[row].size()*50);
		for(size_t col = 0, colsize = _fecGrps[row].size(); col < colsize; ++col){
			// update
			_fecGrps[row][col].updateValue( _fecGrps[row][col].gate()->getSimValue() );
			_fecHash.forceInsert(_fecGrps[row][col]);
		}
		// collect fec grps
		for(size_t j = 0, num = _fecHash.numBuckets(); j < num; ++j){
			if(_fecHash[j].size() > 1){
				for(size_t k =0; k< _fecHash[j].size(); k++){
					_fecHash[j][k].gate()->setGroupNo(newGrps.size());
				}
				newGrps.push_back(_fecHash[j]);
			}
			else if(_fecHash[j].size() == 1)
				_fecHash[j][0].gate()->setGroupNo(-1);
		}
	}       
	_fecGrps.clear();
	_fecGrps = newGrps;
	cout<<"\rTotal #FEC Group = "<<_fecGrps.size()<<std::flush;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
bool CirMgr::checkPattern(string& line){
 	length = line.size();
 	indent = 0;
  	for(size_t i =0, j = line.size();i<j ; ++i){
  		if(line[i] == ' ')
  			indent++;
  		else if(!isdigit(line[i])){
  			cerr<< "Error: Pattern("<<line<<") contains a non-0/1 character('"<<line[i]<<"').\n";
  			cout<<"\r0 patterns simulated.\n";
  			return false;
  		}
  	}
  	if(length-indent != PI_num){
  		cerr<< "Error: Pattern("<<line<<") length("<<length-indent
  		<<") does not match the number of inputs("<<PI_num<<") in a circuit!!\n";
  		cout<<"\r0 patterns simulated.\n";
  		return false;
  	}
  	return true;
}
