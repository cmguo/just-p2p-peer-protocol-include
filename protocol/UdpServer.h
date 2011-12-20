//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

// UdpServer.h

#ifndef _NETWORK_UDP_SERVER_H_
#define _NETWORK_UDP_SERVER_H_

#include <boost/asio/deadline_timer.hpp>

#include <util/RefenceFromThis.h>
#include <protocol/Protocol.h>
#include <struct/UdpBuffer.h>

namespace protocol
{
    struct IUdpServerListener
    {
        typedef IUdpServerListener * p;

        virtual void OnUdpRecv(
            protocol::Packet const & packet) = 0;

        virtual ~IUdpServerListener()
        {
        }
    };

    class UdpServer
        : boost::asio::ip::udp::socket
        , public util::RefenceFromThis<UdpServer>
    {
    public:
        UdpServer(
            boost::asio::io_service & io_svc,
            IUdpServerListener::p handler);

    public:
        bool Listen(
            boost::uint16_t port);

        uint32_t Recv(
            uint32_t count);

        void Close();

        void set_minimal_protocol_verion(boost::uint16_t minimal_protocol_version)
        {
            minimal_protocol_version_ = minimal_protocol_version;
        }

        boost::uint16_t GetUdpPort() const
        {
            if (!is_open())
                return 0;
            return port_;
        }

        template <typename Packet>
        void register_packet();

        void register_all_packets();

        template <typename PacketType>
        bool send_packet(
            PacketType const & packet,
            boost::uint16_t dest_protocol_version);

    protected:
        void UdpRecvFrom(
            UdpBuffer & recv_buffer);

        void UdpSendTo(
            boost::shared_ptr<UdpBuffer> send_buffer,
            boost::uint16_t dest_protocol_version);

        void HandleUdpSendTo(const boost::system::error_code & error,
            uint32_t bytes_transferred, boost::shared_ptr<UdpBuffer> send_buffer);

        void HandleUdpRecvFrom(
            const boost::system::error_code & error,
            uint32_t bytes_transferred,
            UdpBuffer & recv_buffer);

        template <typename Packet>
        void handle_packet(
            UdpBuffer & buffer);

    private:
        typedef void (UdpServer::*packet_handler_type)(
            UdpBuffer & buffer);

        bool get_protocol_version(UdpBuffer & buffer, boost::uint32_t bytes_left,
            boost::uint8_t action, boost::uint16_t & protocol_version);

        bool verify_check_sum(UdpBuffer & buffer, boost::uint32_t chk_sum,
            boost::uint16_t protocol_version);

        IUdpServerListener::p handler_;
        boost::uint16_t port_;
        std::map<boost::uint8_t, packet_handler_type> packet_handlers_;
        boost::uint16_t minimal_protocol_version_;
        boost::uint32_t old_handle_count_;
        boost::uint32_t new_handle_count_;
    };
}

#include "UdpServerHandlePacket.h"

#endif  // _NETWORK_UDP_SERVER_H_
