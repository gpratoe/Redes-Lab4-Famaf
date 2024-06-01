#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

#include <PacketTable.cc>


using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cMessage *tableEvent;
    std::map<int,int> distanceTable;
    std::map<int,int> interfaceByNode;

public:
    Net();
    virtual ~Net();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void updateTables(PacketTable *pkt);
    virtual void sendTableToNeighbour(std::map<int,int> distanceTable);
    virtual std::string printTable(std::map<int,int> table);
};

Define_Module(Net);

#endif /* NET */

Net::Net()
{
}

Net::~Net()
{
    delete(tableEvent);
}

void Net::initialize()
{
    distanceTable[this->getParentModule()->getIndex()] = 0;
    EV << printTable(distanceTable);
    tableEvent = new cMessage("tableEvent");
    scheduleAt(simTime() + 0, tableEvent);
}

void Net::finish()
{
}

void Net::sendTableToNeighbour(std::map<int,int> distanceTable)
{
    
    for (unsigned int i = 0; i < gateSize("toLnk$o"); ++i)
    {
        PacketTable *pkt = new PacketTable();
        pkt->setKind(-1);
        pkt->setDistanceTable(distanceTable);
        send(pkt, "toLnk$o", i);
    }
}

void Net::updateTables(PacketTable *pkt)
{
    std::map<int,int> tableArrived = pkt->getDistanceTable();
    bool updated = false;

    for (std::map<int,int>::iterator it=tableArrived.begin(); it!=tableArrived.end(); ++it)
    {

        if (it->first != this->getParentModule()->getIndex())
        {
            int newValue = tableArrived[it->first] + 1;

            if ((this->distanceTable.find(it->first) == this->distanceTable.end()) || newValue < this->distanceTable[it->first])
            {
                this->distanceTable[it->first] = newValue;
                this->interfaceByNode[it->first] = pkt->getArrivalGate()->getIndex();
                updated = true;
            }
        }
    }
    if (updated)
    {
        sendTableToNeighbour(this->distanceTable);
    }
}

std::string Net::printTable(std::map<int,int> table)
{
    std::string ret = "nodo: " + std::to_string(this->getParentModule()->getIndex()) + "\n";
    for (std::map<int,int>::iterator it=table.begin(); it!=table.end(); ++it)
    {
        ret += "[" + std::to_string(it->first) + "]: " + std::to_string(it->second) + "\n";
    }
    return ret;
}

void Net::handleMessage(cMessage *msg)
{

    // All msg (events) on net are packets
    Packet *pkt = (Packet *)msg;
    PacketTable *pkt_t = (PacketTable *)msg;
    if (msg == tableEvent)
    {
        sendTableToNeighbour(this->distanceTable);
    }
    else if (pkt_t->getKind() == -1)
    {
        updateTables(pkt_t);
        EV << printTable(this->distanceTable);
        delete pkt_t;
        this->bubble("pkt_t dropped");
    }
    // If this node is the final destination, send to App
    else if (pkt->getDestination() == this->getParentModule()->getIndex())
    {
        send(msg, "toApp$o");
    }
    // If not, forward the packet to some else... to who?
    else
    {
        // We send to link interface #0, which is the
        // one connected to the clockwise side of the ring
        // Is this the best choice? are there others?

        int interface = interfaceByNode[pkt->getDestination()];
        pkt->setHopCount(pkt->getHopCount()+1);
        send(msg, "toLnk$o", interface);
    }
}
