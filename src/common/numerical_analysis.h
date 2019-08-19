//!
//! Defines a namespace that can perform numerical analysis of some problem inside it's own thread.
//!
//! A numerical analysis problem is a mathmatical problem that approximates some solution, optimization is a prime example of a numerical analysis.
//! Such a problem is prepared to run forever, producing intermediate solutions (approximations).
//!
//! This namespace exposes three objects that will need to be utilized to do numerical analysis:
//!  - BaseNumericalProblem : Base object that implimentation of problem is to inherit from, the object inheriting this is to run the problem and produce intermidate solutions
//!  - BaseAnalyzer/BaseAnalyzer_IntermediateHistory : Base object that is to determine when to stop, the object inheriting this is to determine when intermediate solutions have converged/diverged
//!  - NumericalAnalysisContainer : Container that is to execute a problem in it's own thread and consult analyzer for termination, also allows timeout to occur when execution has gone too long
//!
//! See example/example_numerical_analysis.h for an example of a problem
//!

#ifndef NUMERICAL_ANALYSIS_H
#define NUMERICAL_ANALYSIS_H


#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <thread>
#include <cmath>
#include <chrono>

namespace NumericalAnalysis
{

//!
//! \brief Defines the possible states of numerical analysis of the series
//!
enum class AnalysisStates
{
    UNKNOWN, // It it not yet known how this series will end up, more data is needed
    CONVERGED, // The series has converged upon a number
    DIVERGED, // The series has gone to infinity, or it has been determined to not converge
    FINISHED // For whatever reason, solution has been found and we are done
};


//!
//! \brief Defines the causation of numerical analysis termination
//!
enum class TerminationTypes
{
    TIMEOUT,    // The problem timed out before a solution could be found
    ABORTED,    // The problem was aborted, likely due to some error
    CONVERGED,  // The problem successfully converged
    DIVERGED    // It was determined that the problem diverges
};


//!
//! \brief Interface for a possible outcome of doing numerical analysis
//!
class IProblemOutcome
{
public:
    virtual TerminationTypes Type() const = 0;
};


//!
//! \brief Outcome given when numerical analysis has resulted in converging upon a number
//!
template <typename SOLUTION_METRICS>
class ProblemOutcome_Converged : public IProblemOutcome
{
private:

    const SOLUTION_METRICS m_solution;

public:

    ProblemOutcome_Converged(const SOLUTION_METRICS &solution) :
        m_solution(solution)
    {

    }

    SOLUTION_METRICS Solution() const
    {
        return m_solution;
    }

    TerminationTypes Type() const
    {
        return TerminationTypes::CONVERGED;
    }
};


//!
//! \brief Outcome given when numerical analysis has diverged
//!
class ProblemOutcome_Diverged : public IProblemOutcome
{
    TerminationTypes Type() const
    {
        return TerminationTypes::DIVERGED;
    }
};


//!
//! \brief Outcome given when numerical analysis has timed out without determining convergance/divergence
//!
class ProblemOutcome_Timeout : public IProblemOutcome
{
    TerminationTypes Type() const
    {
        return TerminationTypes::TIMEOUT;
    }
};


//!
//! \brief Outcome given when numerical analysis has aborted with some error
//!
class ProblemOutcome_Aborted : public IProblemOutcome
{
private:

    const std::string m_Reason;

public:

    ProblemOutcome_Aborted(const std::string &reason) :
        m_Reason(reason)
    {

    }

    std::string Reason() const
    {
        return m_Reason;
    }

    TerminationTypes Type() const
    {
        return TerminationTypes::ABORTED;
    }
};




//!
//! \brief Base Analyzer
//!
//! An Analyzer is to take an intermediate solution and determine the state of the solution, options defined in NumericalAnalysis::SeriesStates.
//!
//! When the solver produces an intermediate solution it is to call NewIntermediateSolution method in this object.
//! That method will in turn call the pure virtual ProcessIntermediateSolution, which is to do analysis.
//!
//! If the solver determines the correct solution outside of optimization, FinalSolution can be called that will perform similar as to a converged series
//!
//! When analysis is done, the outcome is notified by sending to m_ConvergenceNotifier, set by setIntermediateSolutionOutcomeLambda method.
//! There the container of the numerical analysis problem can decide if its time to shutdown the problem, or keep it working.
//!
template <typename SOLUTION_METRICS>
class BaseAnalyzer
{
public:

    //!
    //! Object to notify the entity that is to modify it's behavior according to a SeriesStates as generated by an intermediate output
    //!
    using ISeriesStateNotifier = std::function<void(const SOLUTION_METRICS &solution, const AnalysisStates&)>;


private:

    const ISeriesStateNotifier* m_ConvergenceNotifier;

public:

    BaseAnalyzer() :
        m_ConvergenceNotifier(nullptr)
    {

    }


    //!
    //! \brief Set logic to call when an NumericalAnalysis::SeriesStates has been determined for an intermediate solution
    //! \param obj Object to call
    //!
    void setIntermediateSolutionOutcomeLambda(const ISeriesStateNotifier* obj)
    {
        m_ConvergenceNotifier = obj;
    }


    //!
    //! \brief Give a new intermediate solution to the analyizer.
    //!
    //! Determine the state of the serires and call notifier
    //! \param metrics Intermediate solution
    //!
    void NewIntermediateSolution(const SOLUTION_METRICS &metrics)
    {
        NumericalAnalysis::AnalysisStates state = ProcessIntermediateSolution(metrics);

        if(m_ConvergenceNotifier != nullptr)
        {
            (*m_ConvergenceNotifier)(metrics, state);
        }

    }


    //!
    //! \brief Called if the solver determines it is correct without using the analyzer (evaluation criter)
    //!
    //! This method essentially fast-tracks the solution as FINISHED.
    //! \param metrics Final solution metrics.
    //!
    void FinalSolution(const SOLUTION_METRICS &metrics)
    {
        if(m_ConvergenceNotifier != nullptr)
        {
            (*m_ConvergenceNotifier)(metrics, AnalysisStates::FINISHED);
        }
    }

protected:

    //!
    //! \brief Pure Virtual method to process the solution and detrmine the state of the series.
    //! \param metrics Intermediate solution to process
    //! \return State of the series
    //!
    virtual NumericalAnalysis::AnalysisStates ProcessIntermediateSolution(const SOLUTION_METRICS &metrics) = 0;
};


//!
//! \brief Wrapper for BaseAnalyzer that keeps a history of past solutions.
//!
//! This analyzer can be inherited from if you require a history to do analysis. More convience than anything else.
//!
//! User will need to obstantiate CheckConvergence method and can call upon the protected m_PreviousOutputs member for access to the data.
//!
//! \template SOLUTION_METRICS type of object defining the intermediate solution that the problem is to generate.
//!
template <typename SOLUTION_METRICS>
class BaseAnalyzer_IntermediateHistory : public BaseAnalyzer<SOLUTION_METRICS>
{
private:

    static const uint32_t DEFAULT_OUTPUT_HISTORY = 10;

    const uint32_t m_HistorySize;
    uint64_t m_TotalIntermediateOutputs;

protected:

    std::vector<SOLUTION_METRICS> m_PreviousOutputs;

public:

    //!
    //! \brief Constructor with default output history size
    //!
    BaseAnalyzer_IntermediateHistory() :
        m_HistorySize(DEFAULT_OUTPUT_HISTORY),
        m_TotalIntermediateOutputs(0)
    {

    }


    //!
    //! \brief Constructor setting the size of intermediate history to given scale
    //! \param history Number of intermediate outputs to keep
    //!
    BaseAnalyzer_IntermediateHistory(const uint32_t &history) :
        m_HistorySize(history),
        m_TotalIntermediateOutputs(0)
    {

    }

public:

    //!
    //! \brief Determine the state of the series using history provided in this object
    //! \return State of the Series
    //!
    virtual NumericalAnalysis::AnalysisStates CheckConvergence() = 0;

protected:

    virtual NumericalAnalysis::AnalysisStates ProcessIntermediateSolution(const SOLUTION_METRICS &metrics)
    {
        if(m_PreviousOutputs.size() > m_HistorySize)
        {
            m_PreviousOutputs.erase(m_PreviousOutputs.begin());
        }
        m_PreviousOutputs.push_back(metrics);
        m_TotalIntermediateOutputs++;

        return CheckConvergence();
    }


};


//!
//! \brief Base implimentation of a numerical problem.
//!
//! A numerical problem to be used by NumericalAnalysisContainer is to inherit from this object.
//!
//! The problem itself is to be inserted into Solve method, which is to periodically check IsStopped method to see if it needs to stop
//!
//! The problem can be stopped by calling Stop method.
//!
//! \template ENVIRONMENT Type of object defining the environment the problem is to execute in.
//! \template SOLUTION_METRICS type of object defining the intermediate solution that the problem is to generate.
//!
template <typename ENVIRONMENT, typename SOLUTION_METRICS>
class BaseNumericalProblem
{
private:

    bool m_Stopped;

public:

    //!
    //! \brief Constructor
    //!
    BaseNumericalProblem() :
        m_Stopped(false)
    {

    }


    //!
    //! \brief Perform numerical analysis of a problem, as declared by the obstantiation of this method.
    //!
    //! The problem can go on forever, producing intermediate outputs.
    //! Intermediate or Final outputs are to be given to the analyzer, where convergence/divergence checks are to be done
    //!
    //! \param envirionment Operating parameters for the problem
    //! \param analyzer Object that is to do analysis on progress of the problem
    //! \return Return a string if an error is encountered, empty string if no error is encountered.
    //!
    virtual std::string Solve(const ENVIRONMENT &envirionment, BaseAnalyzer<SOLUTION_METRICS> *analyzer) = 0;


    void Stop()
    {
        m_Stopped = true;
    }

    bool IsStopped() const
    {
        return m_Stopped;
    }
};



//!
//! \brief An object that is to hold a numerical anaysis problem
//!
//! This object will execute the numerical analysis on its own thread, freeing up the host to do other things.
//! When/if an outcome is determined, a lambda function give to Start method will be called to do an async notification
//!
//! This object can be configured to execute the problem for a set amount of time.
//! If execution exceeds that time, then thread will be aborted and host will be notified of computation failure via the lambda provided in START.
//!
//! \template ENVIRONMENT Type of object defining the environment the problem is to execute in.
//! \template SOLUTION_METRICS type of object defining the intermediate solution that the problem is to generate.
//!
template <typename ENVIRONMENT, typename SOLUTION_METRICS>
class NumericalAnalysisContainer
{
private:


    ENVIRONMENT m_CurrentEnvironment;

    bool m_CurrentSolver_Running;
    bool m_CurrentSolver_Finished;

    std::chrono::duration<double> m_Duration;

    std::function<void()> m_CallToAbortCurrentAnalysis;

public:


    NumericalAnalysisContainer(std::chrono::duration<double> duration = std::chrono::seconds(100000000)) :
        m_Duration(duration),
        m_CurrentSolver_Running(false),
        m_CurrentSolver_Finished(false)
    {

    }

    ~NumericalAnalysisContainer()
    {
    }


    //!
    //! \brief Set the environment for the Numerical Analysis Problem
    //!
    //! \throw std::runtime_error Thrown if a numerical problem is currently being solved
    //! \param environment Environment to set
    //!
    void SetEnvironment(const ENVIRONMENT &environment)
    {
        if(m_CurrentSolver_Running == true && m_CurrentSolver_Finished == false)
        {
            throw std::runtime_error("Can not set environment because a problem is currently being solved.");
        }
        m_CurrentEnvironment = environment;

        /// TODO : What should be done if environment is set during problem execution?
    }


    //!
    //! \brief Aborts current problem being solved
    //!
    void StopProblem()
    {
        if(m_CurrentSolver_Running == true && m_CurrentSolver_Finished == false)
        {
            m_CallToAbortCurrentAnalysis();
        }
    }



    //!
    //! \brief Start a numerical analsysis problem
    //! \param problem Problem to execute
    //! \param analyzer Evaluation critera to determine when problem is finished
    //! \param finished Lambda to call when finished, with the problem outcome
    //!
    void Start(
        std::shared_ptr<BaseNumericalProblem<ENVIRONMENT, SOLUTION_METRICS>> problem,
        std::shared_ptr<BaseAnalyzer<SOLUTION_METRICS>> analyzer,
        const std::function<void(std::shared_ptr<IProblemOutcome>)> &finished)
    {

        std::thread timeoutThread([this, problem, finished](){

            auto startTime = std::chrono::system_clock::now();

            /// loop forever
            while(true)
            {
                /// If problem has been solved, this thread is done, exit
                if(m_CurrentSolver_Finished == true)
                {
                    break;
                }

                /// If this thread has exceded it's timeout. Fail
                auto currTime = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = currTime - startTime;
                if(diff > m_Duration)
                {
                    std::shared_ptr<IProblemOutcome> outcome = std::make_shared<ProblemOutcome_Timeout>();
                    m_CurrentSolver_Finished = true;
                    finished(outcome);
                    problem->Stop();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        timeoutThread.detach();

        m_CurrentSolver_Finished = false;
        std::thread solverThread([this, problem, analyzer, finished](){

            m_CurrentSolver_Running = true;

            m_CallToAbortCurrentAnalysis = [this, problem, analyzer, finished](){
                std::shared_ptr<IProblemOutcome> ptr = std::make_shared<ProblemOutcome_Aborted>("Aborted by user");
                m_CurrentSolver_Finished = true;
                finished(ptr);
                problem->Stop();
            };

            typename BaseAnalyzer<SOLUTION_METRICS>::ISeriesStateNotifier processNewSeriesState = [this, problem, analyzer, finished](const SOLUTION_METRICS &solution, const AnalysisStates &state){
                if(state == AnalysisStates::CONVERGED)
                {
                    std::shared_ptr<IProblemOutcome> ptr = std::make_shared<ProblemOutcome_Converged<SOLUTION_METRICS>>(solution);
                    m_CurrentSolver_Finished = true;
                    finished(ptr);
                    problem->Stop();
                }
                if(state == AnalysisStates::FINISHED)
                {
                    std::shared_ptr<IProblemOutcome> ptr = std::make_shared<ProblemOutcome_Converged<SOLUTION_METRICS>>(solution);
                    m_CurrentSolver_Finished = true;
                    finished(ptr);
                    problem->Stop();
                }
                if(state == AnalysisStates::DIVERGED)
                {
                    std::shared_ptr<IProblemOutcome> ptr = std::make_shared<ProblemOutcome_Diverged>();
                    m_CurrentSolver_Finished = true;
                    finished(ptr);
                    problem->Stop();
                }


                return;
            };

            analyzer->setIntermediateSolutionOutcomeLambda(&processNewSeriesState);


            std::string error = problem->Solve(m_CurrentEnvironment, analyzer.get());

            if(m_CurrentSolver_Finished == false)
            {
                std::shared_ptr<IProblemOutcome> outcome = std::make_shared<ProblemOutcome_Aborted>(error);
                finished(outcome);
            }

            m_CurrentSolver_Running = false;
        });
        solverThread.detach();
    }
};

}

#endif // NUMERICAL_ANALYSIS_H
