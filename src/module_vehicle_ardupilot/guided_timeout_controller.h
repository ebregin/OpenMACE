#ifndef GUIDED_TIMEOUT_CONTROLLER_H
#define GUIDED_TIMEOUT_CONTROLLER_H

#include <mavlink.h>

#include <iostream>

#include "data/timer.h"

#include "common/thread_manager.h"
#include "common/class_forward.h"

#include "data_generic_command_item/command_item_components.h"

namespace ardupilot_vehicle{

MACE_CLASS_FORWARD(GuidedTimeoutController);

typedef void(*CallbackFunctionPtr_DynamicTarget)(void*, command_item::Action_DynamicTarget&);

class GuidedTimeoutController : public Thread
{
public:
    GuidedTimeoutController(const unsigned int &timeout);

    ~GuidedTimeoutController() override;

    void start() override;

    void run() override;

    void registerCurrentTarget(const command_item::Action_DynamicTarget &commandTarget);

    void clearTarget();

public:
    void connectTargetCallback(CallbackFunctionPtr_DynamicTarget cb, void *p)
    {
        m_CBTarget = cb;
        m_FunctionTarget = p;
    }

    void callTargetCallback(command_item::Action_DynamicTarget &target)
    {
        if(m_CBTarget != nullptr)
            m_CBTarget(m_FunctionTarget,target);
    }

private:
    CallbackFunctionPtr_DynamicTarget m_CBTarget;
    void *m_FunctionTarget;

private:
    command_item::Action_DynamicTarget m_CurrentTarget;

    Timer m_Timeout;

    unsigned int timeout;

private:
    std::list<std::function<void()>> m_LambdasToRun;

    void clearPendingTasks()
    {
        m_LambdasToRun.clear();
    }

    void RunPendingTasks() {
        while(m_LambdasToRun.size() > 0) {
            auto lambda = m_LambdasToRun.front();
            m_LambdasToRun.pop_front();
            lambda();
        }
    }

};

} //end of namespace ardupilot_vehicle


#endif // STATE_FLIGHT_GUIDED_TIMEOUT_CONTROLLER_H
