% Main simulation script for Heron-UMD task generation.
% A. Wolek, S. Cheng, August 2019
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% MATLAB Initialization
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% check if this is a monte-carlo run, if not clear the workspace
%   preserve single run simulation parameters
%   check MonteCarloEngine.m for details
if ~exist('MonteCarloSwitch','var')
    clear; close all; clc;
    format compact;
    updatePath;
    %rng('default');
    rng(1);
end
rng(1);

% simulate
% temporary fix to allow plotting with time on ROS message callback
% will be replaced with MACE timestamp when available
global tStart;
tStart = tic;

% user should modify loadParams.m as desired for single run
if ~exist('MonteCarloSwitch','var')
    disp('Running standard (non Monte-Carlo) simulation or MACE run')
    %[runParams, ROS_MACE, trueWorld, swarmModel, targetModel] = loadParams_osm();
    [runParams, ROS_MACE, trueWorld, swarmModel, targetModel] = loadParams_cityblocksAtF3();
    % We used loadParams_cityBlocksAtF3 for the flight test at F3 
else
    % for Monte Carlo, specify the IDs of the scenes
    disp('Running Monte-Carlo simulation')
    [runParams, ROS_MACE, trueWorld, swarmModel, targetModel] = loadParams_osm(mapID, algorithmID,initialFormationID,targetMotionID); % IDs are defined in MonteCarloEngine.m
end

% initialize the swarm world, target and swarm states
swarmWorld = initializeSwarmWorld(trueWorld , swarmModel, runParams);
targetState = initializeTargetState(trueWorld, targetModel);
[swarmState, ROS_MACE] = initializeSwarmState(swarmModel, trueWorld, runParams, ROS_MACE);
disp('Run initialized.')

switch runParams.type
    case 'matlab'
        [swarmWorldHist, swarmStateHist, targetStateHist] = simulateMatlab(targetState, targetModel, ...
            swarmState, swarmModel, swarmWorld, trueWorld, runParams);
    case 'mace'
        [swarmWorldHist, swarmStateHist, targetStateHist] = simulateMACE(targetState, targetModel, ...
            swarmState, swarmModel, swarmWorld, trueWorld, runParams, ROS_MACE);
end

% display simulation time
fprintf('Code took %3.3f sec. to run a %3.3f sec. mission (speed = %3.1f X ) \n', toc(tStart), runParams.T,  runParams.T/toc(tStart) );

% save run
pause(1);
if ~exist('MonteCarloSwitch','var')
    % this is the code for single run
    if strcmp(runParams.type,'matlab')
        matFileName = ['runData_' datestr(now,'dd_mmm_yyyy_HHMMSS') '.mat']
    elseif strcmp(runParams.type,'mace')
        matFileName = ['F3FlightData_' datestr(now,'dd_mmm_yyyy_HHMMSS') '.mat']
        diary off
    end
    disp('Saving standard (non Monte-Carlo) simulation');
    save(matFileName,'-v7.3');
else
    % this is for saving Monte Carlo results
    disp('Saving Monte-Carlo simulation');
    matFileName = ['./monteCarloRuns/MonteCarlo_Algorithm' num2str(algorithmID) '_InitialFormation' num2str(initialFormationID) '_mapID' num2str(mapID) '_TargetMotion' num2str(targetMotionID) '.mat']
    save(matFileName,'-v7.3');
end

% play train sound that simulation is done
if (runParams.soundFlag)
load train
sound(y,Fs)
end

% Display Results
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if ~exist('MonteCarloSwitch','var')
    % various movie profiles can be specified here
    %movie_mutualInfoPriors( swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
    movie_targetViews( swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
    movie_mutualInfoWpts( swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
    %movie_lrdt( swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
    
    % plots
    plotPerformance(swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
    %plotOccupGraphTracks(swarmWorldHist, swarmStateHist, targetStateHist, trueWorld, runParams, swarmModel, targetModel )
end
