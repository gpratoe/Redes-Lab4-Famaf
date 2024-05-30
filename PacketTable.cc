#ifndef PACKETTABLE_PKT
#define PACKETTABLE_PKT

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class PacketTable: public cPacket{
private:
    std::vector<int> distanceTable;
    int size;
public:
    PacketTable(){};

     std::vector<int> getDistanceTable(){
        return this->distanceTable;
    };
    
    void setDistanceTable( std::vector<int>distanceTable){
        this->distanceTable = distanceTable;
    };
    
    int getSize(){
        return size;
    };

    void setSize(int size){
        this->size = size;
    }
    
};



#endif