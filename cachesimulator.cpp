/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

struct config{
    unsigned long L1blocksize;
    unsigned long L1setsize;
    unsigned long L1size;
    unsigned long L2blocksize;
    unsigned long L2setsize;
    unsigned long L2size;
};

//** you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
    unsigned long  setIndexBit, tag, blockOffset, indexSize, setSize;
    vector<vector<unsigned long> > myCache;
    vector<vector<bool> > myValid;

public:
    //initializing cache parameters
    cache(unsigned long blockSize, unsigned long setSize, unsigned long cacheSize){

        this->setIndexBit = (unsigned long)log2((cacheSize)*(pow(2,10))/(blockSize*setSize)); // index bits
        this->setSize = setSize; // number of sets/ways
        this->blockOffset = (unsigned long)log2(blockSize); // offset bits
        this->tag = 32 - this->setIndexBit - this->blockOffset; //tag bits
        this->indexSize = (unsigned long)pow(2, this->setIndexBit); // number of index
        this->createCache(setSize, indexSize, blockSize);

    }

    void createCache(unsigned long &setSize, unsigned long &indexSize, unsigned long &blockSize){

        this->myCache.resize(this->getSetSize());
        this->myValid.resize(this->getSetSize());
        for(unsigned long i=0; i<setSize;i++) {
            this->myCache[i].resize(this->getIndexSize());
            this->myValid[i].resize(this->getIndexSize());
        }
    }

    vector<string> getAddress(bitset<32> address) {
        string stringAddr = address.to_string();
        vector<string> addressBits(3);
        addressBits[0] = stringAddr.substr(0, this->tag); //getting tag
        addressBits[1] = stringAddr.substr(this->tag, this->setIndexBit); // getting index
        addressBits[2] = stringAddr.substr(this->tag+this->setIndexBit, this->blockOffset); // getting offset
        return addressBits;
    }

    vector<vector<unsigned long> > &getCache(){
        return this->myCache;
    }

    vector<vector<bool> > &getValid(){
        return this->myValid;
    }

    unsigned long getSetSize(){
        return setSize;
    }

    unsigned long getIndexSize(){
        return this->indexSize;
    }



};
bool tagMatching(cache &myCache, long &index, long tag){

    bool hit= false;
    for(unsigned long i=0;i<myCache.getSetSize();i++){
        if(myCache.getCache()[i][index] == tag && myCache.getValid()[i][index]){
            // hit
            hit=true;
            break;
        }
        else{
            //miss
            hit=false;
        }
    }
    return hit;
}

unsigned long updateCache(cache &myCache, unsigned long &setNumber, long &index, long tag){

    myCache.getCache()[setNumber][index] = (unsigned long)tag;
    myCache.getValid()[setNumber][index] = true;
    return (setNumber+1)%myCache.getSetSize();

}


int main(int argc, char* argv[]){



    config cacheconfig;
    ifstream cache_params;
    string dummyLine;

    string firstLoc = string(argv[1]);
    cache_params.open(firstLoc.c_str());
    while(!cache_params.eof())  // read config file
    {
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L1blocksize;
        cache_params>>cacheconfig.L1setsize;
        cache_params>>cacheconfig.L1size;
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L2blocksize;
        cache_params>>cacheconfig.L2setsize;
        cache_params>>cacheconfig.L2size;
    }



    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like

    if(cacheconfig.L1setsize == 0){
        cacheconfig.L1setsize = cacheconfig.L1size*1024/cacheconfig.L1blocksize;
    }
    if(cacheconfig.L2setsize == 0){
        cacheconfig.L2setsize = cacheconfig.L2size*1024/cacheconfig.L2blocksize;
    }
    cache cache_L1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
    cache cache_L2(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;


    ifstream traces;
    ofstream tracesout;
    string outname;
    string secondLoc = string(argv[2]);
    outname = secondLoc + ".out";

    traces.open(secondLoc.c_str());
    tracesout.open(outname.c_str());

    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;
    bool hit_L1, hit_L2= false;
    vector<unsigned long> counter_L1, counter_L2;
    counter_L1.resize(cache_L1.getIndexSize());
    counter_L2.resize(cache_L2.getIndexSize());



    if (traces.is_open()&&tracesout.is_open()){
        while (getline (traces,line)){   // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            vector<string> addressBitL1 = cache_L1.getAddress(accessaddr);
            string strTagL1 = addressBitL1[0];
            string strIndexL1 = addressBitL1[1];

            vector<string> addressBitL2 = cache_L2.getAddress(accessaddr);
            string strTagL2 = addressBitL2[0];
            string strIndexL2 = addressBitL2[1];

	    char * nllptr;
            long tag_L1 = strtol(strTagL1.c_str(), &nllptr, 2);
            long tag_L2 = strtol(strTagL2.c_str(), &nllptr, 2);

            long index_L1, index_L2;

            if(strIndexL1!=""){
                index_L1 = strtol(strIndexL1.c_str(),&nllptr,2);
            }
            else{
                index_L1 = 0;
            }

            if(strIndexL2!=""){
                index_L2 = strtol(strIndexL2.c_str(),&nllptr,2);
            }
            else{
                index_L2 = 0;
            }


            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0)
            {

                //Implement by you:
                // read access to the L1 Cache,
                //  and then L2 (if required),
                //  update the L1 and L2 access state variable;

                hit_L1 = tagMatching(cache_L1, index_L1, tag_L1);
                if(!hit_L1){
                    hit_L2 = tagMatching(cache_L2, index_L2, tag_L2);
                }

                if(!hit_L1 && !hit_L2){
                    L1AcceState = RM;
                    L2AcceState = RM;
                    counter_L2[index_L2] = updateCache(cache_L2,counter_L2[index_L2], index_L2, tag_L2);
                    // update the L1 cache
                    counter_L1[index_L1] = updateCache(cache_L1,counter_L1[index_L1], index_L1, tag_L1);
                }
                else if(!hit_L1 && hit_L2){
                    L1AcceState = RM;
                    L2AcceState = RH;
                    //put content of l2 into l1
                    counter_L1[index_L1] = updateCache(cache_L1,counter_L1[index_L1], index_L1, tag_L1);

                }
                else if(hit_L1){
                    L1AcceState = RH;
                    L2AcceState = NA;
                }
            }
            else
            {
                //Implement by you:
                // write access to the L1 Cache,
                //and then L2 (if required),
                //update the L1 and L2 access state variable;
                hit_L1 = tagMatching(cache_L1, index_L1, tag_L1);
                if(!hit_L1){
                    hit_L2 = tagMatching(cache_L2, index_L2, tag_L2);
                }

                if (!hit_L1 && !hit_L2) {
                    L1AcceState = WM;
                    L2AcceState = WM;

                    //

                } else if (!hit_L1 && hit_L2) {
                    L1AcceState = WM;
                    L2AcceState = WH;

                    //Do not allocate any space on L1. Pass the write access to L2
                    //Goto L2 cache and update the data array with the new data on that tag


                } else if (hit_L1) {
                    L1AcceState = WH;
                    L2AcceState = NA;
                }
            }
            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else cout<< "Unable to open trace or traceout file ";
    return 0;
}

