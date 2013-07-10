////////////////////////////////////////////////////////////////////////////////
// memTool.cpp tool file for the memTool
// LACC #:  LA-CC-13-051
// Copyright (c) 2013 Michael Mason; HPC-3, LANL
// Copyright (c) 2013 Krell Institute. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
////////////////////////////////////////////////////////////////////////////////


#include <boost/bind.hpp>
#include <typeinfo>
#include <iostream>
#include <stdio.h>
#include <sys/param.h>
#include <string>
#include <sstream>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <cmath>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

// typedefs used by this tool
typedef struct timeval MemTimeValue;
typedef std::string ToolInputArgs;

using namespace KrellInstitute::CBTF;

static void printUsageExample() {
    std::cout << "daemonTool --tool memTool --numBE <number of backends> --toolargs \"mpi-executable-name 45\"" << std::endl;
}

/**
 *
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) MemView :
public Component {

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
                reinterpret_cast<Component*>(new MemView())
                );
    }

private:

    // storage for passed MemTimeValue input
    MemTimeValue elapsedTime;

    /** Default constructor. */
    MemView() :
        Component(Type(typeid(MemView)), Version(1, 0, 0))
    {
	    // input elasped time
	    declareInput<MemTimeValue>(
                "ElapsedTimeIn", boost::bind(&MemView::elapsedTimeHandler, this, _1)
                );
	    // input here is a vector of strings of results.
	    declareInput<std::vector<std::string> >(
                "ResultVecIn", boost::bind(&MemView::resultVecHandler, this, _1)
                );
    }

    void elapsedTimeHandler(const MemTimeValue& in)
    { 
	elapsedTime = in;
    }

    /** Handler for the "in" input.*/
    void resultVecHandler(const std::vector<std::string>& results)
    { 

	MemTimeValue elapsed_time_output;

	// get the output from each backend
	elapsed_time_output = elapsedTime;

	// print each line in the output vector
	for(std::vector<std::string>::const_iterator i = results.begin(); 
	    i != results.end(); ++i) {
	    std::cout << *i << std::endl;
	}

	std::cout << "----------------------" << std::endl;
	std::cout << "Elapsed time: "
	    << elapsed_time_output.tv_sec << "sec "
	    << elapsed_time_output.tv_usec << "microsec"
	    << std::endl;
    }
}; // end class MemView

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(MemView)

/**
 *
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) MemFE :
public Component {

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
                reinterpret_cast<Component*>(new MemFE())
                );
    }

private:

    bool terminate;

    /** Default constructor. */
    MemFE() : Component(Type(typeid(MemFE)), Version(1, 0, 0))
    {
	// input arguments from client (daemonTool).
	declareInput<ToolInputArgs>(
            "args", boost::bind(&MemFE::argsHandler, this, _1)
            );

	// incoming terminate signal
	declareInput<bool> (
            "TerminateIn",
            boost::bind(&MemFE::terminateHandler, this, _1)
            );

	// output for polling frequency.
	declareOutput<struct timespec>("FrequencyOut");
	// output got executable name to monitor.
	declareOutput<std::string>("ExeNameOut");
    }

    /** Handler for the tool "args" input.*/
    void argsHandler(const ToolInputArgs& toolargs)
    { 
	// inititialize termination value
	terminate = false;

	// parse input string
	int freq = 30;
	std::string exe_name = "";
	std::string tmparg = toolargs;
	std::stringstream argstream(tmparg);
	argstream >> exe_name;
	argstream >> freq;

	// default to 30 seconds for polling frequency if invalid value passed.
	if (freq <= 0) {
	    freq = 30;
	}

	// send the polling frequncy downstream.
	struct timespec frequency;
	frequency.tv_sec = (time_t) freq;
	frequency.tv_nsec = 0.0;
        emitOutput<struct timespec>("FrequencyOut", frequency ); 

	// send the executable name to monitor downstream.
	if( exe_name == "" ) {
	    std::cerr << "Error must include vaild proc name" << std::endl;
	    printUsageExample();
	    terminate = true;
	    return;
	} else {
            emitOutput<std::string>("ExeNameOut", exe_name ); 
	}

	// Wait until terminate signal is received before disconnecting from
	// client (daemonTool).
	while(1) {
	    if (terminate == true) break;
	};
    }

    // record the termination signal.
    void terminateHandler(const bool & terminate_signal) {
            terminate = terminate_signal;
    }

}; // end class MemFE

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(MemFE)

/**
 *  * Component type used by the unit test for the Component class.
 *   */
class __attribute__ ((visibility ("hidden"))) getMemInfo :
public Component {

    public:
        /** Factory function for this component type. */
        static Component::Instance factoryFunction()
        {
            return Component::Instance(
                    reinterpret_cast<Component*>(new getMemInfo())
                    );
        }

    private:
        /** Default constructor. */
        getMemInfo() :
            Component(Type(typeid(getMemInfo)), Version(1, 0, 0))
	{
	    // input here is a vector of pids from getPID component.
	    declareInput<std::vector<std::string> >(
                "PidVecIn", boost::bind(&getMemInfo::pidVecInHandler, this, _1)
                );

	    // output for result values to display.
	    declareOutput<std::vector<std::string> >("MemInfoOut");

	    // output for the termination signal.
	    declareOutput<bool>("TerminateOut");
	}

        /** Handler for the "PidVecIn" input.*/
        void pidVecInHandler(const std::vector<std::string>& in)
        { 
            char buffer[100];
            memset(&buffer,0,sizeof(buffer));
            FILE *p = NULL;
            std::string outline = "";
            std::string tmpstr = "";
            std::vector<std::string> output;
            std::string cmd = "";
            float totalmem = 0.0;
            float percentmem = 0.0;
            float pidmem = 0.0;
            char buf[10];
            int result = 0;

            // get hostname
            cmd = "hostname";
            p = popen(cmd.c_str(), "r");
            if(p != NULL) {
                while(fgets(buffer, sizeof(buffer), p) != NULL)
                {
                    outline.assign(buffer);
                    outline = outline.substr(0,outline.rfind("\n"));
                    output.push_back(outline);
                }
                pclose(p);
            } //end if p

            // get total memory for calculation
            cmd = "free -m | grep Mem | awk -F \" \" '{print $2}'";
            p = NULL;
            p = popen(cmd.c_str(), "r");
            if(p != NULL) {
                while(fgets(buffer, sizeof(buffer), p) != NULL)
                {
                    outline.assign(buffer);
                    std::stringstream memStream(outline);
                    memStream >> totalmem;
                }
                pclose(p);
            } //end if p

            // get free
            cmd = "free -m | head -n 2";
            p = NULL;
            p = popen(cmd.c_str(), "r");
            if(p != NULL) {
                while(fgets(buffer, sizeof(buffer), p) != NULL)
                {
                    outline.assign(buffer);
                    outline = outline.substr(0,outline.rfind("\n"));
                    output.push_back(outline);
                }
                pclose(p);
            } //end if p

            output.push_back("pid \t%mem \tmem(MB)");

            // loop over each input pid and perform a ps -o %mem
            for(std::vector<std::string>::const_iterator pid = in.begin(); 
                    pid != in.end(); ++pid) {

                // get %mem from ps ps -p PID -o %mem
                cmd = "ps -p ";
                cmd += *pid;
                cmd += " -o %mem=";

                tmpstr = *pid + " \t";

                p = NULL;
                p = popen(cmd.c_str(), "r");
                if(p != NULL) {
                    while(fgets(buffer, sizeof(buffer), p) != NULL)
                    {
                        outline.assign(buffer);
                        outline = outline.substr(0,outline.rfind("\n"));
                        std::stringstream percentStream(outline);
                        percentStream >> percentmem;
                        percentmem = percentmem / 100;
                    }
                    pclose(p);
                    tmpstr += outline;
                    tmpstr += " \t";
                    //calculate mem from %mem and total mem
                    //!!! totalmem percentmem
                    pidmem = totalmem * percentmem;
                    result = snprintf(buf, 10, "%f", pidmem);
                    tmpstr += buf;
                    tmpstr += "(MB)";
                    output.push_back(tmpstr);
                } //end if p
            } // end for pid

	    // emit the final results...
            emitOutput<std::vector<std::string> >("MemInfoOut", output ); 
        }
}; // end class getMemInfo

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(getMemInfo)

/**
 * Component type used by the unit test for the Component class.
 */
class __attribute__ ((visibility ("hidden"))) getPID :
public Component
{

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
                reinterpret_cast<Component*>(new getPID())
                );
    }

private:
    /** Default constructor. */
    getPID() :
        Component(Type(typeid(getPID)), Version(1, 0, 0))
    {
	// input for executable name to aquire pid(s) for.
        declareInput<std::string>(
            "ExeNameIn", boost::bind(&getPID::exeNameHandler, this, _1)
            );

	// output for pid(s) found.
        declareOutput<std::vector<std::string> >("PidVecOut");
	// output for termination signal.
        declareOutput<bool> ("TerminateOut");
    }

    /** Handler for the "ExeNameIn" input.*/
    void exeNameHandler(const std::string& in)
    { 
        bool terminate;
        char buffer[100];
        FILE *p = NULL;
        FILE *f = NULL;
        std::string outline = "";
        std::vector<std::string> output;

	// build command to find the pid.
        std::string cmd = "ps -u $USER -o pid= -o comm= | grep ";
        cmd += in; 
        cmd += " | awk -F \" \" '{print $1}'";   

        // get results of cmd
        p = popen(cmd.c_str(), "r");
        if(p != NULL) {
            while(fgets(buffer, sizeof(buffer), p) != NULL)
            {
                outline.assign(buffer);
                outline = outline.substr(0,outline.rfind("\n"));
                output.push_back(outline);
            }
            pclose(p);
        } //end if p
        else
        {
            std::cerr << "cannot run cmd = " << cmd << std::endl;
        }

        if(output.size() == 0) {
	    // no output... terminate.
            terminate = true;
        } else {
	    // emit the vector of pid(s).
            emitOutput<std::vector <std::string> >("PidVecOut", output ); 
            terminate = false;
        }

	// emit the termination signal.
        emitOutput<bool>("TerminateOut", terminate);
    }
}; // end class getPID

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(getPID)

class __attribute__((visibility("hidden"))) BeginCircutLogic:
public Component { 
    public:
        /** Factory function for this component type. */
        static Component::Instance factoryFunction() {
            return Component::Instance(
                    reinterpret_cast<Component*>(new BeginCircutLogic())
                    );
        }

    private:
        struct timespec frequency;
        std::string exename_to_monitor;
        /** Default constructor. */
        BeginCircutLogic() :
            Component(Type(typeid(BeginCircutLogic)), Version(1, 0, 0)) {
                frequency.tv_sec = 0;
                frequency.tv_nsec = 0;
                exename_to_monitor = "";
                declareInput<std::string>(
                        "ExeNameIn",
                        boost::bind(&BeginCircutLogic::exeNameHandler,
                                this,
                                _1)
                        );
                declareInput<struct timespec>(
                        "FrequencyIn",
                        boost::bind(&BeginCircutLogic::frequencyHandler,
                                this,
                                _1)
                        );
                declareInput<bool>(
                        "RestartIn",
                        boost::bind(&BeginCircutLogic::restartHandler,
                                this,
                                _1)
                        );
                declareOutput<std::string>("ExeNameOut");
                declareOutput<MemTimeValue>("StartTimeOut");
            }

        void exeNameHandler(const std::string & fn) {
            exename_to_monitor = fn;
            if (frequency.tv_sec != 0 || frequency.tv_nsec !=0) {
	    //std::cerr << "BeginCircutLogic::exeNameHandler emits exeName to monitor " << exename_to_monitor << std::endl;
            //emitOutput<std::string>("ExeNameOut", exename_to_monitor);
	    //std::cerr << "BeginCircutLogic::exeNameHandler calls start_circuit" << std::endl;
                start_circut();
	    }
        }

        void restartHandler(const bool & restart) {
            if (restart) {
                start_circut();
	    }
        }

        void frequencyHandler(const struct timespec & freq) {
            frequency.tv_sec = freq.tv_sec;
            frequency.tv_nsec = freq.tv_nsec;
            if (exename_to_monitor != "") {
                start_circut();
	    }
        }

        void start_circut() {
            MemTimeValue start_time;
            gettimeofday(&start_time, NULL);
            emitOutput<MemTimeValue>("StartTimeOut", start_time);
            nanosleep(&frequency, NULL);
            emitOutput<std::string>("ExeNameOut", exename_to_monitor);
        }
};

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(BeginCircutLogic)

class __attribute__((visibility("hidden"))) EndCircutLogic:
public Component {
    public:
        static Component::Instance factoryFunction() {
            return Component::Instance(
                    reinterpret_cast<Component*>(new EndCircutLogic())
                    );
        }

    private:
        MemTimeValue start_time;
        bool terminate;
        EndCircutLogic() :
            Component(Type(typeid(EndCircutLogic)), Version(1, 0, 0)) {
		// init terminate.
                terminate = false;
                declareInput<MemTimeValue>(
                        "StartTimeIn",
                        boost::bind(&EndCircutLogic::startTimeHandler,
                                this,
                                _1)
                        );
                declareInput<std::vector <std::string> >(
                        "MemoryInfoIn",
                        boost::bind(&EndCircutLogic::memoryInfoHandler, this, _1)
                        );
                declareInput<bool> (
                        "TerminateIn",
                        boost::bind(&EndCircutLogic::terminateHandler, this, _1)
                        );
                declareOutput<std::vector <std::string> >("MemoryInfoOut");
                declareOutput<MemTimeValue>("ElapsedTimeOut");
                declareOutput<bool>("RestartOut");
                declareOutput<bool>("TerminateOut");
            }

        void startTimeHandler(const MemTimeValue & start) {
            start_time.tv_sec = start.tv_sec;
            start_time.tv_usec = start.tv_usec;
        }

        void memoryInfoHandler(const std::vector<std::string> & mem_info) {
            MemTimeValue elapsed_time = getElapsedTime();
            emitOutput<MemTimeValue>("ElapsedTimeOut", elapsed_time);
            if (!terminate) {
                emitOutput<bool>("RestartOut", true);
	    }
            emitOutput<std::vector <std::string> >("MemoryInfoOut", mem_info);
        }

        void terminateHandler(const bool & terminate_signal) {
            terminate = terminate_signal;
            emitOutput<bool>("TerminateOut", terminate);
        }

        MemTimeValue getElapsedTime() {
            long sec;
            long usec;
            MemTimeValue elapsed_time;
            MemTimeValue end_time;
            gettimeofday(&end_time, NULL);
            sec = end_time.tv_sec - start_time.tv_sec;
            if ((usec = end_time.tv_usec - start_time.tv_usec) < 0) {
                usec += 1000000;
                sec -= 1;
                if(sec < 0) {
                    std::cerr << "Elapsed time is negative." << std::endl;
                    sec = 0;
                    usec = 0;
                }
            }
            elapsed_time.tv_sec = sec;
            elapsed_time.tv_usec = usec;
            return elapsed_time;
        }
};

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(EndCircutLogic)
