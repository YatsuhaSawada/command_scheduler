// "Copyright [year] <Copyright Owner>"
#pragma once

//===========================================
template<class Result>
class Command_T {
 public:
// lifeCycle
    Command_T() {}
    Command_T(const Command_T& rhs) = delete;

// action
    virtual Result execute() = 0;
    virtual Result cancel() = 0;

// operator overload
    Command_T& operator=(const Command_T& rhs) = delete;
    Command_T& operator=(Command_T&&) = default;
};

//===========================================
template<class Result, class Action, class Cancel>
class Command
    : public Command_T<Result> {
 public:
// lifeCycle
    Command(Action action, Cancel cancel);

// action
    Result execute() final;
    Result cancel() final;

 private:
    Action _action;
    Cancel _cancel;
};


template< class Result, class Action, class Cancel >
Command< Result, Action, Cancel >::Command(Action action, Cancel cancel)
    : _action(action)
    , _cancel(cancel)
{
}

template< class Result, class Action, class Cancel >
Result Command< Result, Action, Cancel >::execute()
{
    Result result = _action();

    return result;
}

template< class Result, class Action, class Cancel >
Result Command< Result, Action, Cancel >::cancel()
{
    return _cancel();
}

//===========================================
template< class Result, class Action, class Cancel >
Command_T<Result>* createCommand(Action action, Cancel cancel)
{
    typedef Command< Result, Action, Cancel > Obj;

    return new Obj(action, cancel);
}