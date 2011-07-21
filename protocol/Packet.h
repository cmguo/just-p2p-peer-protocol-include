//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// Packet.h

#ifndef _PROTOCOL_PACKET_H_
#define _PROTOCOL_PACKET_H_

#include "struct/Structs.h"
#include "struct/Base.h"

namespace protocol
{
    const boost::uint16_t PEER_VERSION_V3 = 0x0103;
    const boost::uint16_t PEER_VERSION_V4 = 0x0104;
    const boost::uint16_t PEER_VERSION_V5 = 0x0105;
    const boost::uint16_t PEER_VERSION_V6 = 0x0106;
    const boost::uint16_t PEER_VERSION_V7 = 0x0107;
    const boost::uint16_t PEER_VERSION = PEER_VERSION_V7;

    struct Packet
    {
        static uint32_t NewTransactionID()
        {
            static uint32_t glocal_transaction_id = 0;
            glocal_transaction_id ++;
            return glocal_transaction_id;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ar & transaction_id_;
        }

        virtual boost::uint32_t length() const
        {
            return sizeof(transaction_id_);
        }

        virtual ~Packet(){}

        boost::asio::ip::udp::endpoint end_point;
        boost::uint8_t PacketAction;
        uint32_t transaction_id_;
    };

    template <boost::uint8_t action>
    struct PacketT
    {
        virtual ~PacketT(){}
        static const boost::uint8_t Action = action;
    };

}

#endif  // _PROTOCOL_PACKET_H_
