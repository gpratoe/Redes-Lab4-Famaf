#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

#include <PacketTable.cc>

#define FAKE_INFINITY __INT_MAX__

using namespace omnetpp;

class Net : public cSimpleModule
{
private:
    cMessage *tableEvent;
    std::vector<int> distanceTable;
    std::vector<int> interfaceByNode;

public:
    Net();
    virtual ~Net();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void updateTables(PacketTable *pkt);
    virtual void sendTableToNeighbour(std::vector<int> distanceTable);
    virtual std::string printTable(std::vector<int> table);
};

Define_Module(Net);

#endif /* NET */

Net::Net()
{
}

Net::~Net()
{
}

void Net::initialize()
{

    distanceTable.resize(this->getParentModule()->getVectorSize(), FAKE_INFINITY);
    interfaceByNode.resize(this->getParentModule()->getVectorSize(), -1);
    distanceTable[this->getParentModule()->getIndex()] = 0;
    EV << printTable(distanceTable);
    tableEvent = new cMessage("tableEvent");
    scheduleAt(simTime() + 0, tableEvent);
}

void Net::finish()
{
}

void Net::sendTableToNeighbour(std::vector<int> distanceTable)
{
    
    for (unsigned int i = 0; i < gateSize("toLnk$o"); ++i)
    {
        cGate* gate =  cModule::gate("toLnk$o",i);

        PacketTable *pkt = new PacketTable();
        pkt->setKind(1);
        pkt->setDistanceTable(distanceTable);
        pkt->setSize(distanceTable.size());
        if(gate->isConnected()){
            send(pkt, "toLnk$o", i);
        }
    }
}

void Net::updateTables(PacketTable *pkt)
{
    std::vector<int> tableArrived = pkt->getDistanceTable();
    bool updated = false;

    for (unsigned int i = 0; i < pkt->getSize(); i++)
    {

        if ((tableArrived[i] != FAKE_INFINITY) && (i != this->getParentModule()->getIndex()))
        {
            int newValue = tableArrived[i] + 1;

            if (newValue < this->distanceTable[i])
            {
                this->distanceTable[i] = newValue;
                this->interfaceByNode[i] = pkt->getArrivalGate()->getIndex();
                updated = true;
            }
        }
    }
    if (updated)
    {
        sendTableToNeighbour(this->distanceTable);
    }
}

std::string Net::printTable(std::vector<int> table)
{
    std::string ret = "nodo: " + std::to_string(this->getParentModule()->getIndex()) + "\n";
    for (unsigned int i = 0; i < table.size(); i++)
    {
        ret += "[" + std::to_string(i) + "]: " + std::to_string(table[i]) + "\n";
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
    else if (pkt_t->getKind() == 1)
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
        send(msg, "toLnk$o", interface);
    }
}
