/**
 * @file isotp_sender.cpp
 *
 * This file holds a basic sender and receiver class to transmit data over CAN
 * using the ISO-TP protocol. The received data is handled by the 
 * `proceedReceivedData()`-function. To do something useful with the received
 * information, derive a new class from this one and override 
 * `proceedReceivedData()` with the new data handling (e.g. a routine to send
 * a proper response).
 */

#include "isotp_socket.h"
#include "service_identifier.h"
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <array>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <iomanip>

using namespace std;

constexpr size_t MAX_BUFSIZE = 5000; ///< >= 4096 bytes

/**
 * Opens the ISO_TP socket for sending.
 *
 * @return 0 on success, otherwise a negative value
 * @see IsoTpSocket::closeSender()
 */
int IsoTpSocket::openSender() noexcept
{
    struct sockaddr_can addr;
    addr.can_addr.tp.tx_id = source_; // sender
    addr.can_addr.tp.rx_id = dest_; // receiver
    addr.can_family = AF_CAN;

    int skt = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    if (skt < 0)
    {
        cerr << __func__ << "() socket: " << strerror(errno) << '\n';
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, device_.c_str(), device_.length() + 1);
    ioctl(skt, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;

    auto bind_res = bind(skt,
                         reinterpret_cast<struct sockaddr*>(&addr),
                         sizeof(addr));
    if (bind_res < 0)
    {
        cerr << __func__ << "() bind: " << strerror(errno) << '\n';
        close(skt);
        return -2;
    }
    send_skt_ = skt;

    return 0;
}

/**
 * Closes the ISO_TP socket for sending.
 * 
 * @see IsoTpSocket::openSender()
 */
void IsoTpSocket::closeSender() noexcept
{
    if (send_skt_ < 0)
    {
        cerr << __func__ << "() Sender socket is already closed!\n";
        return;
    }
    close(send_skt_);
    send_skt_ = -1;
}

/**
 * Send the given number of bytes located in the buffer. The sender socket
 * has to be opened first.
 * 
 * @param buffer: the pointer to the data buffer
 * @param size: the number of bytes to write in the socket
 * @return the number of sent bytes or a negative value on error
 * @see IsoTpSocket::openSender()
 * @see IsoTpSocket::closeSender()
 */
int IsoTpSocket::sendData(const void* buffer, size_t size) noexcept
{
    if (send_skt_ < 0)
    {
        cerr << __func__ << "() Invalid socket file descriptor!\n";
        return -1;
    }

    auto bytes_sent = write(send_skt_, buffer, size);
    if (bytes_sent < 0)
    {
        cerr << __func__ << "() write: " << strerror(errno) << '\n';
        return -2;
    }

    cout << __func__ << "() Sent " << dec << bytes_sent << " bytes." << endl;
    return bytes_sent;
}

/**
 * Opens the socket for receiving the via ISO_TP transmitted data.
 *
 * @return 0 on success, otherwise a negative value
 * @see IsoTpSocket::closeReceiver()
 */
int IsoTpSocket::openReceiver() noexcept
{
    isOnExit_ = false;
    struct sockaddr_can addr;
    addr.can_addr.tp.tx_id = source_;
    addr.can_addr.tp.rx_id = dest_;

    addr.can_family = AF_CAN;

    int skt = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    if (skt < 0)
    {
        cerr << __func__ << "() socket: " << strerror(errno) << '\n';
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, device_.c_str(), device_.length() + 1);
    ioctl(skt, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;

    auto bind_res = bind(skt,
                         reinterpret_cast<struct sockaddr*>(&addr),
                         sizeof(addr));
    if (bind_res < 0)
    {
        cerr << __func__ << "() bind: " << strerror(errno) << '\n';
        close(skt);
        return -2;
    }

    receive_skt_ = skt;
    return 0;
}

/**
 * Closes the socket for receiving data.
 * 
 * @see IsoTpSocket::openReceiver()
 */
void IsoTpSocket::closeReceiver() noexcept
{
    isOnExit_ = true;

    if (receive_skt_ < 0)
    {
        cerr << __func__ << "() Receiver socket is already closed!\n";
        return;
    }
    close(receive_skt_);
    receive_skt_ = -1;
}

/**
 * Reads the data from the opened receiver socket. The handling of the received
 * data is controlled by the inside nested `proceedReceivedData()` function. To 
 * change its behavior, deduce a new class form `IsoTpSocket` and override the
 * `proceedReceivedData()`-function to your likings.
 * 
 * @return 0 on success, otherwise a negative value
 * @see IsoTpSocket::proceedReceivedData()
 * @see IsoTpSocket::closeReceiver()
 */
int IsoTpSocket::readData() noexcept
{
    if (receive_skt_ < 0)
    {
        cerr << __func__ << "() Can not read data. Receiver socket invalid!\n";
        return -1;
    }

    uint8_t msg[MAX_BUFSIZE];
    size_t num_bytes;
    do
    {
        num_bytes = read(receive_skt_, msg, MAX_BUFSIZE);
        if (num_bytes > 0 && num_bytes < MAX_BUFSIZE)
        {
            proceedReceivedData(msg, num_bytes);
        }
    }
    while (!isOnExit_);

    return 0;
}

/**
 * Proceeds the received data. This is the default implementation, which simply
 * prints out the received data in hexadecimal notation to `std::out`. This 
 * function is supposed be overridden to do something useful with the received 
 * data. To do this, derive an new class from `IsoTpSocket` and override the 
 * function to your likings.
 * 
 * @see IsoTpSocket::readData()
 */
void IsoTpSocket::proceedReceivedData(const uint8_t* buffer, size_t num_bytes) noexcept
{
    cout << __func__ << "() Received " << dec << num_bytes << " bytes.\n";
    for (unsigned i = 0; i < num_bytes; i++)
    {
        cout << " 0x"
                << hex
                << setw(2)
                << setfill('0')
                << static_cast<int> (buffer[i]);
    }
    cout << endl;
}
