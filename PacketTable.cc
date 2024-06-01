#ifndef PACKETTABLE_PKT
#define PACKETTABLE_PKT

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class PacketTable: public cPacket{
private:
    std::map<int,int> distanceTable;
public:
    PacketTable(){};

     std::map<int,int> getDistanceTable(){
        return this->distanceTable;
    };
    
    void setDistanceTable( std::map<int,int>distanceTable){
        this->distanceTable = distanceTable;
    };
};



#endif
