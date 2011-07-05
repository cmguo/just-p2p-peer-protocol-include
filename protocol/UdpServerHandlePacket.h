//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// UdpServerHandlePacket.h

#ifndef _NETWORK_UDP_SERVER_HANDLE_PACKET_H_
#define _NETWORK_UDP_SERVER_HANDLE_PACKET_H_

#include "protocol/PeerPacket.h"

#include <util/archive/LittleEndianBinaryIArchive.h>
#include <util/archive/LittleEndianBinaryOArchive.h>

#include <iostream>

namespace protocol
{

    typedef util::archive::LittleEndianBinaryIArchive<boost::uint8_t> IUdpArchive;
    typedef util::archive::LittleEndianBinaryOArchive<boost::uint8_t> OUdpArchive;

    template <typename PacketType>
    void UdpServer::register_packet()
    {
        boost::uint8_t action = PacketType::Action;
        packet_handlers_[action] =
            &UdpServer::handle_packet<PacketType>;
    }

    template <typename PacketType>
    void UdpServer::handle_packet(
        UdpBuffer & recv_buffer)
    {
        IUdpArchive ia(recv_buffer);
        PacketType packet;
        packet.end_point = recv_buffer.end_point();
        ((protocol::Packet &)packet).PacketAction = PacketType::Action;
        ia >> packet;
        if (ia) 
        {
            handler_->OnUdpRecv(packet);
        }
        else
        {
        //    assert(false);
        }
    }

    template <typename PacketType>
    bool UdpServer::send_packet(
        PacketType const & packet,
        boost::uint16_t dest_protocol_version)
    {
        UdpBuffer send_buffer;

        assert(boost::asio::ip::udp::endpoint() != packet.end_point);
        send_buffer.end_point(packet.end_point);
        send_buffer.commit(sizeof(uint32_t));
        OUdpArchive oa(send_buffer);
        boost::uint8_t action = PacketType::Action;
        oa << action << packet;
        if (oa)
        {
            UdpSendTo(send_buffer, dest_protocol_version);
        }
        return oa;
    }
}

#endif  // _NETWORK_UDP_SERVER_HANDLE_PACKET_H_
