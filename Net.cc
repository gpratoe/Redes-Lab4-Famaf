#ifndef NET
#define NET

#include <string.h>
#include <list>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

struct tableEntry{
    int node;
    int distance;
    std::string interface;
};

class Net: public cSimpleModule {
private:
    cMessage *initHelloEvent;
    std::vector<struct tableEntry> nodeTable;
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    initHelloEvent = new cMessage("InitHello");
    scheduleAt(simTime() + 0, initHelloEvent);
}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;
    if(msg == initHelloEvent){ //Send all the hello´s to the neighbours
        for(unsigned int i=0; i<gateSize("toLnk$o"); ++i){
            Packet *hello = new Packet("Hello", 1); //1 is the kind of "Hello"
            hello->setSource(this->getParentModule()->getIndex());
            send(hello, "toLnk$o", i);
        }
        
        
    }
    else if (pkt->getKind() == 1){ //Ensure the hello is send once and handle it
        std::string outGate = "toLnk$o[" + std::to_string(pkt->getArrivalGate()->getIndex()) + "]";
        struct tableEntry tabEnt = {pkt->getSource(), 1, outGate};
        nodeTable.push_back(tabEnt); //Add to the routing table the neighbour
        
        EV << nodeTable[0].node << " vecino de " << this->getParentModule()->getIndex();
        delete pkt;
        this->bubble("hello dropped");
    }
    // If this node is the final destination, send to App
    else if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        send(msg, "toApp$o");
    }
    // If not, forward the packet to some else... to who?
    else {
        // We send to link interface #0, which is the
        // one connected to the clockwise side of the ring
        // Is this the best choice? are there others?
        send(msg, "toLnk$o", 0);
    }
}
