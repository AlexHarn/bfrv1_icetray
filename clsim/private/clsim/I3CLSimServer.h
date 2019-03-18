
#ifndef CLSIM_I3CLSIMSERVER_H_INCLUDED
#define CLSIM_I3CLSIMSERVER_H_INCLUDED

#include "icetray/I3Logging.h"
#include "clsim/I3CLSimStep.h"
#include "clsim/I3CLSimStepToPhotonConverter.h"

#include <thread>
#include <future>
#include <memory>
#include <map>
#include <list>
#include <queue>
#include <zmq.hpp>

class I3CLSimServer {
public:
    I3CLSimServer(const std::string &address, const std::vector<I3CLSimStepToPhotonConverterPtr> &converters);
    ~I3CLSimServer();

    std::map<std::string, double> GetStatistics() const;
private:
    std::vector<I3CLSimStepToPhotonConverterPtr> converters_;
    std::size_t workgroupSize_, maxBunchSize_;
    
    zmq::context_t context_;
    zmq::socket_t control_, heartbeat_;

    SET_LOGGER("I3CLSimServer");

    void ServerThread(const std::string &bindAddress);
    void WorkerThread(unsigned index, unsigned buffer_id);

    std::thread serverThread_;
    std::vector<std::thread> workerThreads_;
};

/// @brief Client for communicating with I3CLSimServer
///
/// None of the methods are thread-safe, but EnqueueSteps/EnqueueBarrier
/// and GetConversionResultWithBarrierInfo may be called from different threads
/// to feed and drain the client asynchronously.
class I3CLSimClient {
public:
    I3CLSimClient(const std::string &server_address);
    ~I3CLSimClient();

    /// @brief Submit steps for propagation
    ///
    /// This function will block until the server is ready to handle more steps
    void EnqueueSteps(I3CLSimStepSeriesConstPtr steps, uint32_t identifier);
    /// @brief Flush any steps held by the server
    ///
    /// This function will block until the server is ready to handle more steps
    void EnqueueBarrier();
    /// @brief Retrieve propagated photons from the next step bunch
    ///
    /// Bunches will be returned in the order they were enqueued.
    /// barrierWasJustReset will be set to `true` when the last bunch enqueued
    /// before the barrier is returned.
    ///
    /// This function will block until results are available
    I3CLSimStepToPhotonConverter::ConversionResult_t GetConversionResultWithBarrierInfo(bool &barrierWasJustReset);

    std::size_t GetWorkgroupSize() const { return workgroupSize_; }
    std::size_t GetMaxNumWorkitems() const { return maxBunchSize_; }
    
private:
    zmq::context_t context_;
    zmq::socket_t inbox_, outbox_, control_;
    std::size_t workgroupSize_, maxBunchSize_;

    SET_LOGGER("I3CLSimClient");

    std::future<void> clientTask_;
};

#endif // CLSIM_I3CLSIMSERVER_H_INCLUDED

