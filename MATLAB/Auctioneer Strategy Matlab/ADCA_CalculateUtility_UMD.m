function [agentBid, concludingState, feasible] = ADCA_CalculateUtility_UMD(roboticAgent, currentTime, biddingTask, previousState)

%{
    Variable roboticAgent:
    Variable currentTime:
    Variable previousTask: Expects a ADCA_TaskContainer object that
    contains any of the previous tasks currently assigned to the robotic
    agent. Allowed to remain empty if there are no previous tasks.
%}
feasible = false; %By default, assume that the agent cannot address the task within the time window

nominalVelocity = roboticAgent.m_RoboticParameters.NominalVelocity;
fuelCost = roboticAgent.m_RoboticParameters.FuelCost;  % Sheng: Fuel cost is a value of cost for each distance unit.

%evaluate the properties relating to the task, during this phase it should
%generate the beginning state, final state, and the inner task properties

%The following logic should eventually be packed up on a per agent/vehicle
%evaluation basis. For now, we will evaluate this here to check the logic
%and math.

transitionalState = roboticState(); % Sheng: just initialize the variable transitionalState.
transitionalState.POSITION = previousState.POSITION;
transitionalState.TIME = previousState.TIME;

startingState = roboticState();
concludingState = roboticState();

if(biddingTask.m_TaskDescriptor.getTaskType() == ADCA_TaskTypes.TASK_LOITER)
    loiterLocation = biddingTask.m_TaskDescriptor.LoiterLocation;
    
    startingState.POSITION = loiterLocation;
    concludingState.POSITION = loiterLocation;
    
elseif(biddingTask.m_TaskDescriptor.TASK_TYPE == ADCA_TaskTypes.TASK_SURVEY)
end

taskLocation = startingState.POSITION;
requiredStartTime = biddingTask.m_TaskDescriptor.Time_RequiredStart;


transitionDistanceWarped = sqrt((transitionalState.POSITION.x-taskLocation.x)^2 + ...
    (transitionalState.POSITION.y-taskLocation.y)^2 + ...
    (transitionalState.POSITION.z-taskLocation.z)^2);
transitionTimeWarped = transitionDistanceWarped/nominalVelocity;

agentState = roboticAgent.m_CurrentState;
transitionDistancOptimistic = sqrt((agentState.POSITION.x-taskLocation.x)^2 + ...
    (agentState.POSITION.y-taskLocation.y)^2 + ...
    (agentState.POSITION.z-taskLocation.z)^2);
transitionTimeOptimistic = transitionDistancOptimistic/nominalVelocity;

% Sheng: transitionDistanceMAX is really the distance between task location
% and current location of the agent.

minStartWarped = max(requiredStartTime, transitionalState.TIME + transitionTimeWarped); %Sheng: the latter one of the requiredStartTime and the sum of transitionalState's TIME and time of transition
minStartOptimistic = max(requiredStartTime, transitionalState.TIME + transitionTimeOptimistic); %Sheng: the latter one of the requiredStartTime and the sum of transitionalState's TIME and time of transition

%     if((requiredStartTime ~= 0) && (minStart >= requiredStartTime))
%         return;
%     end


%Compute the expected duration of the task based on the estimated
%start time and the expected task duration as required from the robotic
%agent.
expectedTaskDuration = roboticAgent.getExpectedTaskDuration(biddingTask);

maxEndWarped = minStartWarped + expectedTaskDuration; % worst case (latest) of the time when this task can be finished
maxEndOptimistic= minStartOptimistic + expectedTaskDuration; % worst case (latest) of the time when this task can be finished

rewardWarped = biddingTask.m_TaskDescriptor.BaseReward *...
    exp(-biddingTask.m_TaskDescriptor.Time_Penalty *...
    (minStartWarped-requiredStartTime)); %+lengthOfQueue^0.99

rewardOptimistic = biddingTask.m_TaskDescriptor.BaseReward *...
    exp(-biddingTask.m_TaskDescriptor.Time_Penalty *...
    (minStartOptimistic-requiredStartTime)); %+lengthOfQueue^0.99

% reward = 100;

% Sheng: reward is the BaseReward discounted by an exponential factor
% proportional to the delayed time between requiredStartTime and worst case
% of start time (transitionalState.TIME + transitionTime).

%To ensure that the rules of Diminishing Marginal Gains are followed we
%have to ensure that the maximum penalty parameter is applied
workWarped = fuelCost * (transitionDistanceWarped + biddingTask.TaskDistance);
workOptimistic = fuelCost * (transitionDistancOptimistic + biddingTask.TaskDistance);

% work = transitionDistance;
% work is the total fuel cost for the agent to arrive at the task location
% and complete the task.
penaltyOptimistic = workOptimistic; %For the time being, penalty and work is 1:1

agentBid = ADCA_BidDescriptor();
agentBid.updateBidProperties(roboticAgent.m_RoboticParameters.AGENT_ID,...
    biddingTask.m_TaskDescriptor.getTaskKey(),workOptimistic,penaltyOptimistic,rewardOptimistic,...
    currentTime);

startingState.TIME = minStartOptimistic;
concludingState.TIME = maxEndOptimistic;

agentBid.updateStateProperties(transitionalState, startingState, concludingState);
agentBid.WARPED_UTILITY = rewardWarped - workWarped;
feasible = true; %if we have gotten to this point we will just assume that an agent can address the task
end