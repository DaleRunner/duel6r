/*
 * Service.cpp
 *
 *  Created on: Mar 19, 2020
 *      Author: fanick
 */
#include "Service.h"
#include "binarystream/BinaryStream.h"
#include "Message.h"
#include "Object.h"
#include "Peer.h"
namespace Duel6::net {

    void Service::setMasterAddressAndPort(const std::string &address, int port){
        masterServerProxy.setAddressAndPort(address, port);
    }
    void Service::requestStop() {
        stopRequested = true;
    }
    void Service::flush() {
        enet_host_flush(serviceHost.get());
    }
    void Service::poll(Float64 elapsedTime, Uint32 timeout) {
        if (!(state == ServiceState::STARTED || state == ServiceState::STARTING || state == ServiceState::STOPPING)) {
            return;
        }

        runPeriodicalTasks(elapsedTime);

        ENetEvent event;
        ENetHost *netHost = serviceHost.get();
        bool serviced = false;
        do {
            // enet_host_service interacts with the socket (receive commands, send waiting commands)
            if (!serviced && enet_host_service(netHost, &event, timeout) <= 0) break;
            serviced = true;
            switch (event.type) {
            case ENetEventType::ENET_EVENT_TYPE_NONE:
                break;
            case ENetEventType::ENET_EVENT_TYPE_CONNECT:
                onConnected(event.peer, event.data);
                break;
            case ENetEventType::ENET_EVENT_TYPE_DISCONNECT:
                onDisconnected(event.peer, event.data);
                break;
            case ENetEventType::ENET_EVENT_TYPE_RECEIVE:
                onReceived(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;
            }
            if (state == ServiceState::UNINITIALIZED) {
                return;
            }
            // enet_host_check_events does the same as enet_host_service, but does not interact with the socket
        } while (enet_host_check_events(netHost, &event) > 0);

        if(stopRequested){
            stop();
            stopRequested = false;
        }
    }

    void Service::onMasterServerConnected(ENetPeer *peer) {
        masterServerProxy.onConnected(peer);
    }

    void Service::onMasterServerDisconnected(ENetPeer* peer, enet_uint32 reason) {
        masterServerProxy.setPeer(nullptr);
        delete (PeerRef*) peer->data;
    }

    void Service::onMasterServerReceived(ENetPeer *peer, ENetPacket *packet) {
        masterServerProxy.onReceived(peer, packet);
    }

    void Service::onPeerReceived(ENetPeer *peer, ENetPacket *packet) {
        PeerRef *ref = (PeerRef*) peer->data;
        recordPeerNetStats(peer);
        binarystream bs(packet->data, packet->dataLength);
        MessageType type = MessageType::MAX_COUNT;
        if (!(bs >> type)) {
            D6_THROW(Exception, "Service::onPeerReceived");
        }
        switch (type) {
        case MessageType::OBJECT: {
            ObjectType objectType;
            if (!((bs >> objectType) == true)) {
                D6_THROW(Exception, "Cannot deserialize MessageType::OBJECT");
            }
            ref->peer->handle(objectType, bs);
            break;
        }
        case MessageType::EVENT: {
            EventType eventType;
            if (!(bs >> eventType) == true) {
                D6_THROW(Exception, "Cannot deserialize MessageType::EVENT");
            }
            ref->peer->handle(eventType, bs);
            break;
        }
        case MessageType::MAX_COUNT:
            default:
            D6_THROW(Exception, "Unexpected deserialization")
            ;
            break;
        }
    }

    void Service::onConnected(ENetPeer *peer, enet_uint32 data) {
        REQUEST_TYPE requestType;
        if (data >= static_cast<unsigned int>(REQUEST_TYPE::COUNT)) {
            peer->data = nullptr;
            enet_peer_disconnect_now(peer, 0x66666666);
            return;
        }
        requestType = static_cast<REQUEST_TYPE>(data);

        if(requestType == REQUEST_TYPE::MASTER_PUSH_NAT_PEERS_TO_SERVER){ // incoming request
            std::cout << "MASTER_PUSH_NAT_PEERS_TO_SERVER\n";
            Duel6::net::PeerRef *ref = new Duel6::net::PeerRef();
            peer->data = ref;
            ref->isMasterServer = true;
            ref->peer = nullptr;
            ref->pos = 0;
            return onMasterServerConnected(peer);
        }
        if(requestType == REQUEST_TYPE::NONE){ // TODO outgoing request
            if(peer->data != nullptr){
                PeerRef * ref = (PeerRef *) peer->data;
                if(ref->isMasterServer){
                    return onMasterServerConnected(peer);
                } else {
                    return onPeerConnected(peer);
                }
            } else {
                return onPeerConnected(peer);
            }
        }

        if (requestType == REQUEST_TYPE::GAME_CONNECTION) { // incoming request
            return onPeerConnected(peer);
        } else {
            D6_THROW(Exception, "Unexpected request type in onConnected ");
        }
    }

    void Service::setLocalIPAddress(const std::string &localIPAddress) {
        this->localIPAddress = localIPAddress;
    }

}
