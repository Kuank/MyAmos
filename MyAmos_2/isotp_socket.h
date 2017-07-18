/**
 * @file isotp_socket.h
 *
 */

#ifndef ISOTP_SOCKET_H
#define ISOTP_SOCKET_H

#include <cstddef>
#include <string>
#include <thread>
#include <linux/can.h>

class IsoTpSocket
{
public:
    IsoTpSocket() = delete;

    IsoTpSocket(canid_t source,
                canid_t dest,
                const std::string& device)
    : source_(source)
    , dest_(dest)
    , device_(device)
    {
    }
    virtual ~IsoTpSocket() = default;
    int openSender() noexcept;
    void closeSender() noexcept;
    int sendData(const void* buffer, std::size_t size) noexcept;
    int openReceiver() noexcept;
    int readData() noexcept;
    void closeReceiver() noexcept;
    
protected:
    virtual void proceedReceivedData(const std::uint8_t* buffer,
                                     std::size_t num_bytes) noexcept;

private:
    canid_t source_;
    canid_t dest_;
    const std::string device_;
    int send_skt_ = -1;
    int receive_skt_ = -1;
    bool isOnExit_ = false;

};

#endif /* ISOTP_SOCKET_H */
