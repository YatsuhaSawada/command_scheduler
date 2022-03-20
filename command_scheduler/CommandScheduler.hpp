//  "Copyright [year] <Copyright Owner>"
#pragma once

#include <string>
#include <list>
#include <mutex>

#include "Command.hpp"

// ===========================================
template<class Result>
class CommandScheduler {
 public:
// lifeCycle
    CommandScheduler();
    ~CommandScheduler();
    CommandScheduler(const CommandScheduler& rhs) = delete;
    CommandScheduler(CommandScheduler&& rhs) = default;
// ===
    bool start(const std::string threadName);
    void stop();

    int64_t add(Command_T<Result>* pCommand);
    bool empty();

    Result cancelCurrent();
    Result cancelAll();

// operator overload
    CommandScheduler & operator=(const CommandScheduler& rhs) = delete;
    CommandScheduler& operator=(CommandScheduler&&) = default;

 private:
    void run();

 private:
    std::mutex _mutex;

    bool _notify;
    std::mutex _waitMutex;
    std::condition_variable _waitObject;

    int64_t _seq;
    Command_T<Result>*             _pCurrent;
    std::list<Command_T<Result>*>* _pReqestQueue;
    std::thread*           _pThread;
};

// ===========================================
// lifeCycle
// ===========================================
template< class Result >
CommandScheduler< Result >::CommandScheduler()
    : _mutex()
    , _notify(false)
    , _waitMutex()
    , _waitObject()
    , _seq(0)
    , _pReqestQueue(new std::list<Command_T<Result>*>())
    , _pThread(nullptr) {
}

// ===========================================
template< class Result >
CommandScheduler< Result >::~CommandScheduler()
{
    stop();
}

// ===========================================
//
// ===========================================
template< class Result >
bool CommandScheduler< Result >::start(const std::string threadName) {
    _pThread = new std::thread(&CommandScheduler< Result >::run, this);
    if (nullptr == _pThread) {
        return false;
    }

    return true;
}

// ===========================================
template< class Result >
void CommandScheduler< Result >::stop() {
    if (nullptr == _pThread) {
        return;
    }

    _notify = true;
    _waitObject.notify_all();

    _pThread->join();

    delete _pThread;
    _pThread = nullptr;
}

// ===========================================
template< class Result >
int64_t CommandScheduler< Result >::add(Command_T<Result>* pCommand) {
    std::unique_lock<std::mutex> lock(_mutex);

    _pReqestQueue->push_back(pCommand);

    _notify = true;
    _waitObject.notify_all();

    return _seq++;
}

// ===========================================
template< class Result >
Result CommandScheduler< Result >::cancelCurrent() {
    std::unique_lock<std::mutex> lock(_mutex);

    if (nullptr == _pCurrent) {
        return Result(Result::OK);
    }

    return _pCurrent->cancel();
}

// ===========================================
template< class Result >
Result CommandScheduler< Result >::cancelAll()
{
    std::unique_lock<std::mutex> lock(_mutex);

    FOR_BY_ITR(p, *_pReqestQueue) {
        delete (*p);
    }

    return _pCurrent->cancel();
}

// ===========================================
template< class Result >
bool CommandScheduler< Result >::empty() {
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    if (!lock) {
        return false;
    }

    if (nullptr != _pCurrent) {
        return false;
    }

    return _pReqestQueue->empty();
}

// ===========================================
// private
// ===========================================
template< class Result >
void CommandScheduler< Result >::run() {
    for (;;) {
        if (0 == _pReqestQueue->size()) {
            std::unique_lock<std::mutex> lk(_waitMutex);
            _notify = false;
            _waitObject.wait(lk, [&] { return _notify; });
        }

        {
            std::unique_lock<std::mutex> lock(_mutex);

            auto p = _pReqestQueue->begin();
            if (p == _pReqestQueue->end()) {
                break;
            }
            _pCurrent = *(p);
            if (!_pCurrent) {
                break;
            }
            _pReqestQueue->pop_front();
        }

        _pCurrent->execute();

        delete _pCurrent;
        _pCurrent = nullptr;
    }
}
