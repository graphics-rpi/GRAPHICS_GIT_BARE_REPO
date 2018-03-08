#ifndef _LI_THREAD_DISPATCHER_HXX_
#define _LI_THREAD_DISPATCHER_HXX_

#include "LiPrerequisites.hxx"

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/version.hpp>
#include <boost/function.hpp>
#include <deque>

#include "boost_version.hxx"

#ifdef BOOST_DEPRECATED_THREAD_MODEL
#include <boost/thread/condition.hpp>
#else
#include <boost/thread/condition_variable.hpp>
#endif

namespace LI {
  class ThreadDispatcher {
  public:
    typedef boost::function<void (const ThreadDispatcher*)> Callback;

  private:
    class Command {
    public:
      Command(ThreadDispatcher* w);
      virtual void execute() const = 0;

    protected:
      //! This is a const pointer to a non-const ThreadDispatcher.
      ThreadDispatcher* const mThreadDispatcher;

    private:
      Command();
    };

    // TODO: Make StepCommand not ambiguous between busy run and stepping.
    class StepCommand : public Command {
    public:
      StepCommand(ThreadDispatcher* w);
      void execute() const;
    };

    class SetCallbackCommand : public Command {
    public:
      SetCallbackCommand(ThreadDispatcher* w, const Callback& cb = 0);
      void execute() const;

    private:
      Callback mNewCallback;
    };

    class SetModeCommand : public Command {
    public:
      SetModeCommand(ThreadDispatcher* w, bool value);
      void execute() const;

    private:
      bool mNewBusyRunMode;
    };

    class QuitCommand : public Command {
    public:
      QuitCommand(ThreadDispatcher* w);
      void execute() const;
    };

    friend class QuitCommand;
    friend class SetCallbackCommand;
    friend class SetModeCommand;
    friend class StepCommand;

  public:
    explicit ThreadDispatcher();
    virtual ~ThreadDispatcher();

    bool isBusyRunning() const;
    unsigned int getQueueSize() const;
    unsigned long getFrameCount() const;

    unsigned int prod();
    void forceQuit();
    //! Sets the mode which the ThreadDispatcher uses to call mRunCallback.
    //! If mBusyRun is true, it will keep calling it constantly while there
    //! is a callback set. If it is false, it will draw a frame every time
    //! it is prodded.
    void setBusyRun(bool value);
    void setInitCallback(const Callback& cb);
    void setRunCallback(const Callback& cb);
    void unsetRunCallback();

  protected:
    void pushCommand(const boost::shared_ptr<Command>& command);
    void run();

    bool mBusyRun;
    bool mKick;
    bool mQuit;
    unsigned long mFrameCount;
    Callback mRunCallback;

    mutable boost::mutex mQueueMutex;
#ifdef BOOST_DEPRECATED_THREAD_MODEL
    boost::condition mQueueCond;
#else
    boost::condition_variable mQueueCond;
#endif
    std::deque<boost::shared_ptr<Command> > mCommandQueue;

    mutable boost::mutex mInitCallbackMutex;
#ifdef BOOST_DEPRECATED_THREAD_MODEL
    boost::condition mCallbackCond;
#else
    boost::condition_variable mCallbackCond;
#endif
    Callback mInitCallback;

    boost::scoped_ptr<boost::thread> mThread;
  };

  // Inline definitions below.














































































  inline ThreadDispatcher::Command::Command(ThreadDispatcher* w)
    : mThreadDispatcher(w) {}

  inline ThreadDispatcher::StepCommand::StepCommand(ThreadDispatcher* w)
    : Command(w) {}

  inline void ThreadDispatcher::StepCommand::execute() const {
    if(mThreadDispatcher->mBusyRun) {
      if(!mThreadDispatcher->mRunCallback) {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
        boost::mutex::scoped_lock lock(mThreadDispatcher->mQueueMutex);
#else
        boost::unique_lock<boost::mutex> lock(mThreadDispatcher->mQueueMutex);
#endif
        while(mThreadDispatcher->mCommandQueue.back().get() == this) {
          mThreadDispatcher->mQueueCond.wait(lock);
        }
        return;
      }

#ifdef EBUG
      std::cerr << "Busy run start frame: " << mThreadDispatcher->mFrameCount << std::endl;
#endif
      while(mThreadDispatcher->mCommandQueue.back().get() == this) {
        mThreadDispatcher->mRunCallback(mThreadDispatcher);
        ++mThreadDispatcher->mFrameCount;
      }
#ifdef EBUG
      std::cerr << "Busy run end frame:   " << mThreadDispatcher->mFrameCount << std::endl;
#endif
    } else {
      mThreadDispatcher->mRunCallback(mThreadDispatcher);
      ++mThreadDispatcher->mFrameCount;
    }
  }

  inline ThreadDispatcher::SetCallbackCommand::SetCallbackCommand(ThreadDispatcher* w, const ThreadDispatcher::Callback& cb)
    : Command(w),
      mNewCallback(cb) {}

  inline void ThreadDispatcher::SetCallbackCommand::execute() const {
    mThreadDispatcher->mRunCallback = mNewCallback;
  }

  inline ThreadDispatcher::SetModeCommand::SetModeCommand(ThreadDispatcher* w, bool value)
    : Command(w),
      mNewBusyRunMode(value) {}

  inline void ThreadDispatcher::SetModeCommand::execute() const {
    mThreadDispatcher->mBusyRun = mNewBusyRunMode;
  }

  inline ThreadDispatcher::QuitCommand::QuitCommand(ThreadDispatcher* w)
    : Command(w) {}

  inline void ThreadDispatcher::QuitCommand::execute() const {
    mThreadDispatcher->mQuit = true;
  }

  inline ThreadDispatcher::ThreadDispatcher()
    : mBusyRun(false),
      mKick(false),
      mQuit(false),
      mFrameCount(1),
      mThread(new boost::thread(boost::bind(&ThreadDispatcher::run, this))) {}

  inline ThreadDispatcher::~ThreadDispatcher() {
#ifdef EBUG
    std::cerr << "ThreadDispatcher dtor." << std::endl;
#endif

    forceQuit();

    mThread->join();
#ifdef EBUG
    std::cerr << "Thread joined." << std::endl;
#endif
  }

  inline bool ThreadDispatcher::isBusyRunning() const {
    return mBusyRun;
  }

  inline unsigned int ThreadDispatcher::getQueueSize() const {
    return mCommandQueue.size();
  }

  inline unsigned long ThreadDispatcher::getFrameCount() const {
    return mFrameCount;
  }

  inline unsigned int ThreadDispatcher::prod() {
    unsigned int ret = mCommandQueue.size();
    pushCommand(boost::shared_ptr<Command>(new StepCommand(this)));
    return ret;
  }

  inline void ThreadDispatcher::forceQuit() {
    {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
      boost::mutex::scoped_lock lock(mQueueMutex);
#else
      boost::lock_guard<boost::mutex> lock(mQueueMutex);
#endif
      while(mCommandQueue.size() > 1) {
        mCommandQueue.pop_back();
      }
    }
    pushCommand(boost::shared_ptr<Command>(new QuitCommand(this)));
  }

  inline void ThreadDispatcher::setBusyRun(bool value) {
    pushCommand(boost::shared_ptr<Command>(new SetModeCommand(this, value)));
  }

  inline void ThreadDispatcher::setInitCallback(const Callback& cb) {
    {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
      boost::mutex::scoped_lock lock(mInitCallbackMutex);
#else
      boost::lock_guard<boost::mutex> lock(mInitCallbackMutex);
#endif
      mInitCallback = cb;
    }
    mCallbackCond.notify_one();
  }

  inline void ThreadDispatcher::setRunCallback(const Callback& cb) {
    pushCommand(boost::shared_ptr<Command>(new SetCallbackCommand(this, cb)));
  }

  inline void ThreadDispatcher::unsetRunCallback() {
    pushCommand(boost::shared_ptr<Command>(new SetCallbackCommand(this)));
  }

  inline void ThreadDispatcher::pushCommand(const boost::shared_ptr<Command>& command) {
    {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
      boost::mutex::scoped_lock lock(mQueueMutex);
#else
      boost::lock_guard<boost::mutex> lock(mQueueMutex);
#endif
      mCommandQueue.push_back(command);
    }
    mQueueCond.notify_one();
  }

  inline void ThreadDispatcher::run() {
#ifdef EBUG
    std::cerr << "Run!" << std::endl;
#endif

    // Wait for init callback to be set.
    {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
      boost::mutex::scoped_lock lock(mInitCallbackMutex);
#else
      boost::unique_lock<boost::mutex> lock(mInitCallbackMutex);
#endif
      while(!mInitCallback && !mQuit) {
        mCallbackCond.wait(lock);
      }

      if(mQuit) {
#ifdef EBUG
        std::cerr << "Told to quit before init callback was called." << std::endl;
#endif
        return;
      }

      mInitCallback(this);
    }

    while(!mQuit) {
      {
        bool should_prod = false;
        {
#ifdef BOOST_DEPRECATED_THREAD_MODEL
          boost::mutex::scoped_lock lock(mQueueMutex);
#else
          boost::unique_lock<boost::mutex> lock(mQueueMutex);
#endif
          if(mCommandQueue.empty()) {
            if(mBusyRun) {
              should_prod = true;
            } else {
              while(mCommandQueue.empty()) {
                mQueueCond.wait(lock);
              }
            }
          }
        }

        if(should_prod) {
          prod();
        }
      }

      mCommandQueue.front()->execute();
#ifdef BOOST_DEPRECATED_THREAD_MODEL
      boost::mutex::scoped_lock lock(mQueueMutex);
#else
      boost::lock_guard<boost::mutex> lock(mQueueMutex);
#endif
      mCommandQueue.pop_front();
    }
  }
}
#endif
