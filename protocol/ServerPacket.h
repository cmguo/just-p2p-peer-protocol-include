//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// ServerPacket.h

#ifndef _PROTOCOL_SERVER_PACKET_H_
#define _PROTOCOL_SERVER_PACKET_H_

#include "protocol/Packet.h"

namespace protocol
{

    struct ServerPacket
        : Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & IsRequest;
            if (IsRequest) {
                ar & peer_version_;
                ar & reserve_;
            } else {
                ar & error_code_;
            }
        }

        ServerPacket()
        {
            IsRequest = 1;
            peer_version_ = PEER_VERSION;
            reserve_ = 0;
        }

        boost::uint8_t IsRequest;
        
        boost::uint16_t reserve_;
        boost::uint16_t peer_version_;
        boost::uint8_t error_code_;
    };

    template <boost::uint8_t action>
    struct ServerPacketT
        : PacketT<action>
        , ServerPacket
    {
    };

}

#endif  // _PROTOCOL_SERVER_PACKET_H_