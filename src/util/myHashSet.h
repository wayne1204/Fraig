/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      friend class TaskMgr;
      iterator(vector<Data>* buc,size_t num, size_t r): _buckets(buc),_numBuckets(num), row(r), pos(0) {
         _data = &_buckets[row][pos];
      }
      ~iterator() {}
      const Data& operator * () const { return *_data; }
      Data& operator * () { return *_data; }
      iterator& operator ++ () { 
         if(pos +1 == _buckets[row].size()){
            row ++;
            pos = 0;
            while( _buckets[row].empty() && row < _numBuckets){
               row++;
            }
            _data = &_buckets[row][pos];
         }
         else{
            pos++;
            _data++;
         } 
         return (*this); 
      }
      iterator operator ++ (int) { 
         iterator tmp = *this;
         operator ++();
         return tmp; 
      }
      iterator& operator -- () { 
         if(pos -1 < 0){
            row --;
            while( _buckets[row].empty() && row > 0){
               row--;
            }
            pos = _buckets[row].size()-1;
            _data = &_buckets[row][pos];
         }
         else{
            pos --;
            _data --;
         }
         return (*this); 
      }
   iterator operator -- (int) { 
      iterator tmp = *this;
      operator --();
      return tmp; 
   }

   iterator& operator = (const iterator i){
      _buckets = i._buckets;
      _data = i._data;
      row = i.row;
      pos = i.pos;
      return *(this);
   }
   bool operator == (const iterator& i) const { return (_data == i._data); }
   bool operator != (const iterator& i) const { return (_data != i._data); }
   private:
      vector<Data>* _buckets;
      Data* _data;
      size_t _numBuckets;
      size_t row;
      size_t pos;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }
   // vector<Data>& at(size_t i) { return _buckets[i]; }
   // const vector<Data>& at(size_t i) const { return _buckets[i]; }   
   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
      iterator begin() const { 
         for(size_t i =0; i<_numBuckets; ++i){
            if(!_buckets[i].empty())
               return iterator( _buckets,_numBuckets, i); 
         }
        return iterator( _buckets,_numBuckets,_numBuckets); 
    }
   // Pass the end
    iterator end() const { 
        return iterator(_buckets,_numBuckets,_numBuckets);
    }
   // return true if no valid data
   bool empty() const { return (size() == 0 ); }
   // number of valid data
   size_t size() const { 
      size_t s = 0; 
      for (size_t i = 0; i < _numBuckets; ++i) {
         s += _buckets[i].size();
      }
      return s; 
   }
   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const { 
      size_t pos = ( bucketNum(d));
      for (size_t i =0, k =_buckets[pos].size(); i<k; i++){
         if( _buckets[pos][i] == d){
            return true;
         }
      }
      return false; 
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const {
      size_t pos = ( bucketNum(d));
      for (size_t i =0, k =_buckets[pos].size(); i<k; i++){
         if( _buckets[pos][i] == d){
            d = _buckets[pos][i];
            return true;
         }
      }
      return false; 
   }
   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) { 
      size_t pos = ( bucketNum(d));
      for (size_t i =0, k =_buckets[pos].size(); i<k; i++){
         if( _buckets[pos][i] == d){
            _buckets[pos][i] = d;
            return true;
         }

      }
      _buckets[ bucketNum(d) ].push_back(d);
      return false; 
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) { 
      if( check(d) == true )  return false;
         // cout<<"inserting"<<d.gate()->getID()<<"into Hash "<<bucketNum(d)<<endl;
         _buckets[ bucketNum(d) ].push_back(d);
      return true; 
   }
   void forceInsert( const Data& d){
      // cout<<"force inserting"<<d.gate()->getID()<<"into Hash "<<bucketNum(d)<<endl;
      _buckets[ bucketNum(d) ].push_back(d);
   }
   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) { 
   if( check(d) == false )  return false;
   size_t pos = ( bucketNum(d));
   for(size_t i =0, k = _buckets[pos].size(); i<k; ++i){
      if( _buckets[pos][i] == d )
         _buckets[pos][i] = _buckets[pos][_buckets[pos].size()-1];
   }
   _buckets[pos].resize( _buckets[pos].size()-1 );
   return true; 
   }
private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
     return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
