////////////////////////////////////////////////////////////////////////////////
// psPlugin.cpp defines components for the psTool
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
#include <boost/lexical_cast.hpp>
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
#include <algorithm>

#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/SignalAdapter.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>

// typedefs used by this tool

// input argument string from the daemonTool client.
typedef std::string ToolInputArgs;

// Used to differentiate the std::vector<std::string> input
// to the psView component.
// cbtf will assign a separate stream to each type below.
typedef std::vector<std::string> sameVec;
typedef std::vector<std::string> diffVec;

using namespace KrellInstitute::CBTF;

static void printUsageExample() {
    std::cout << "daemonTool --tool pstool --numBE <number of backends>" << std::endl;
}


class __attribute__ ((visibility ("hidden"))) psFE :
    public Component
{

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
          reinterpret_cast<Component*>(new psFE())
        );
    }

private:

    bool terminate;  // when true, shutdown tool.

    /** Default constructor. */
    psFE() :
        Component(Type(typeid(psFE)), Version(1, 0, 0))
    {
	// input arguments from client (daemonTool).
	declareInput<ToolInputArgs>(
            "args", boost::bind(&psFE::argsHandler, this, _1)
            );

	// incoming terminate signal
	declareInput<bool> (
            "TerminateIn",
            boost::bind(&psFE::terminateHandler, this, _1)
            );

        declareOutput<std::string >("stringout");
    }

    /** Handler for the tool "args" input.*/
    void argsHandler(const ToolInputArgs& toolargs)
    { 
	// inititialize termination value
	terminate = false;

	// This is here in case the this tool would like to allow passing
	// options to the psCmd component at the backend.
	// parse input string
	//std::string tmparg = toolargs;
	//std::stringstream argstream(tmparg);

	// Sending this string down stream kicks of the ps command on all
	// backends.
	//std::cerr << "INPUT ON argsHandler is " << toolargs << std::endl;
        emitOutput<std::string>("stringout", "start" ); 

	// Wait until terminate signal is received before disconnecting from
	// client (daemonTool).  (wait a quarter of a second).
	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = 250000000L;
	while(!terminate) {
	    nanosleep(&tim , &tim2);
	};
    }
 
    // record the termination signal.
    void terminateHandler(const bool & terminate_signal) {
	terminate = terminate_signal;
    }

}; // end class psFE

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(psFE)

/**
 *
 * Component type used to view final results.
 */
class __attribute__ ((visibility ("hidden"))) psView :
public Component {

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
                reinterpret_cast<Component*>(new psView())
                );
    }

private:

    bool same_finished, diff_finished;
    int numSame, numDiff, nodes;
    std::vector<std::string> sameoutput, diffoutput;

    /** Default constructor. */
    psView() :
        Component(Type(typeid(psView)), Version(1, 0, 0))
    {
        declareInput<sameVec>(
            "sameVecIn", boost::bind(&psView::sameVecInHandler, this, _1)
            );
        declareInput<diffVec>(
            "diffVecIn", boost::bind(&psView::diffVecInHandler, this, _1)
            );
        declareOutput<bool>("finished");

	// when both of these are true, one of the output handlers will
	// emit the termination notification to psFE.
	same_finished = false;
	diff_finished = false;
        numSame = 0;
        numDiff = 0;
        nodes = Impl::TheTopologyInfo.NumChildren;
        sameoutput.clear();
        diffoutput.clear();

    }

    /** Handler for the "sameVecIn" input.*/
    void sameVecInHandler(const sameVec& results)
    { 
      // bump. only emit output when all nodes have replied.
      numSame++;
      nodes = Impl::TheTopologyInfo.NumChildren;

      // save the ps output for this node
      for(sameVec::const_iterator i = results.begin(); i != results.end(); ++i)
      {
	if (std::find(sameoutput.begin(), sameoutput.end(), *i) == sameoutput.end()) {
	   sameoutput.push_back(*i);
	}
      }


     if (numSame >= nodes) {
	std::cout << "---List of  Same Proc's---" << std::endl;
	for(std::vector<std::string>::iterator si = sameoutput.begin();
	    si != sameoutput.end(); ++si) {
	    std::cout << *si;
	}
	std::cout << "---End Same Proc's---" << std::endl;;

	same_finished = true;
	if (same_finished && diff_finished) {
            emitOutput<bool>("finished", true ); 
	}
      }
    }

    /** Handler for the "diffVecIn" input.*/
    void diffVecInHandler(const diffVec& results)
    { 
      nodes = Impl::TheTopologyInfo.NumChildren;
      // bump. only emit output when all nodes have replied.
      numDiff++;

      // save the ps output for this node
      for(diffVec::const_iterator i = results.begin(); i != results.end(); ++i)
      {
	if (std::find(diffoutput.begin(), diffoutput.end(), *i) == diffoutput.end()) {
	  diffoutput.push_back(*i);
	}
      }

      if (numDiff >= nodes) {
	std::cout << "---List of  Diff Proc's---" << std::endl;
	for(std::vector<std::string>::iterator si = diffoutput.begin();
	    si != diffoutput.end(); ++si) {
	    std::cout << *si;
	}

	std::cout << "---End Diff Proc's---" << std::endl;;
	diff_finished = true;
	if (same_finished && diff_finished) {
            emitOutput<bool>("finished", true ); 
	}
      }
    }

}; // end class psView

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(psView)


/**
 *  * Filter to find the same proc's
 *   */
class __attribute__ ((visibility ("hidden"))) psSame :
    public Component
{

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
          reinterpret_cast<Component*>(new psSame())
        );
    }

private:
    // variables
    int runNum;
    int nodes;
    sameVec outVec;
    std::vector<std::vector<std::string> > nodeVec;

    /** Default constructor. */
    psSame() :
        Component(Type(typeid(psSame)), Version(1, 0, 0))
    {
        declareInput<std::vector <std::string> >(
            "in", boost::bind(&psSame::inHandler, this, _1)
            );
        declareOutput<sameVec>("out");

        // node is set to the number of children below the node
        // this component is running on. (based on mrnet topology info). 
        runNum = 0;
        nodes = Impl::TheTopologyInfo.NumChildren;
        outVec.clear();
        nodeVec.clear();
    }

    /** Handler for the "in" input.*/
    void inHandler(const std::vector<std::string>& in)
    { 
      //reset the outVec on the first run
      //can't reset it after you emitoutput because because it is bound to it
      if(runNum == 0) {
        outVec.clear();
        nodeVec.clear();
      }

      // bump. only emit output when all nodes have replied.
      runNum++;

      // If there is only one node, then all procs are the same.
      if (nodes == 1) {
        emitOutput<sameVec>("out", in);
	return;
      }

      // save the ps output for this node
      nodeVec.push_back(in);

      std::string noderank;

      // once all nodes have responded search through the nodeVec to see
      // what procs are the same on all nodes
      if(runNum >= nodes) {
        // check if a proc on one node is running on any other
        // grab each vector of procs
        int found;
        for(std::vector<std::vector<std::string> >::const_iterator
               procVec = nodeVec.begin();
               procVec != nodeVec.end();
               ++procVec)
        {
          // take one nodes vector of procs.
          for(std::vector<std::string>::const_iterator proc = procVec->begin(); 
	        proc != procVec->end(); ++proc)
          {
            found = 0;
	    if (proc == procVec->begin()) {
	       noderank = *proc;
	       //std::cerr << "procVec TOP " << noderank << std::endl;
	    }

            // for all the other vector of procs
            for(std::vector<std::vector<std::string> >::const_iterator
                    procVecOther = procVec;
                    procVecOther != nodeVec.end();
                    ++procVecOther)
            {
              // don't check itself
              if(procVecOther != procVec)
              {
                found = 0;
                // look at each of the procs in the other vectors
                for(std::vector<std::string>::const_iterator
                        procOther = procVecOther->begin();
                        procOther != procVecOther->end();
                        ++procOther)
                {
                // check to see if the original proc is the same as any other
                // if it is stop looking
                if(*proc == *procOther)
                {
                    found = 1;
                    break;
                  } 
                }  //procOther

                // if the proc was found then continue checking the others
                // else don't bother searching the others
                if(found == 0) 
                {
                  break;
                }
	      } // if procVecOther != procVec
            } //procVecOther

            // after looking through all the procs on the other nodes
            // if found is 1 then it was found on every node
            // if found is 0 then it was not on every node
            if(found) 
            {
              // this proc was found on every node so save it
              // but don't save it twice
	      if (std::find(outVec.begin(), outVec.end(), *proc) == outVec.end()) {
                // this proc hasn't been saved yet so save it
                outVec.push_back(*proc);
	      }
	    } // end found

          } // proc
        } //procVec

        // output the outVec
	if (outVec.size() > 0 ) {
            emitOutput<sameVec>("out", outVec);
	} else {
	    outVec.push_back("");
            emitOutput<sameVec>("out", outVec);
	}
        //reset the counter
        runNum = 0;
      }
    }
}; // end class psSame

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(psSame)


/**
 *  * Filter to fine the diff proc's
 *   */
class __attribute__ ((visibility ("hidden"))) psDiff :
    public Component
{

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
          reinterpret_cast<Component*>(new psDiff())
        );
    }

private:
    // variables
    int runNum;
    int nodes;
    diffVec outVec;
    std::vector<std::vector<std::string> > nodeVec;

    /** Default constructor. */
    psDiff() :
        Component(Type(typeid(psDiff)), Version(1, 0, 0))
    {
        declareInput<std::vector <std::string> >(
            "in", boost::bind(&psDiff::inHandler, this, _1)
            );
        declareOutput<diffVec>("out");

        // node is set to the number of chlidren below the node
        // this component is running on. (based oon mrnet topology info). 
        runNum = 0;
        nodes = Impl::TheTopologyInfo.NumChildren;
	bool IsFrontend = Impl::TheTopologyInfo.IsFrontend;
        outVec.clear();
        nodeVec.clear();
    }

    /** Handler for the "in" input.*/
    void inHandler(const std::vector<std::string>& in)
    { 
      //reset the outVec on the first run
      //can't reset it after you emitoutput because because it is bound to it
      if(runNum == 0) {
        outVec.clear();
        nodeVec.clear();
      }

      // bump. only emit output when all nodes have replied.
      runNum++;

      // if there is only one node, then there are no differences
      // so send an empty diffVec and return;
      if (nodes == 1) {
	outVec.push_back("");
        emitOutput<diffVec>("out", outVec);
	return;
      }

      // save the ps output for this node
      nodeVec.push_back(in);

      // once all nodes have responded search through the nodeVec to see
      // what procs are the same on all nodes
      if(runNum >= nodes) {
        // check if a proc on one node is running on any other
        // grab each vector of procs
        int found;
        for(std::vector<std::vector<std::string> >::const_iterator
                procVec = nodeVec.begin();
                procVec != nodeVec.end();
                ++procVec)
        {
          // take one proc
          for(std::vector<std::string>::const_iterator proc = procVec->begin(); 
	        proc != procVec->end(); ++proc)
          {
            found = 0;
            // for all the other vector of procs
            for(std::vector<std::vector<std::string> >::const_iterator
	           procVecOther = nodeVec.begin();
                   procVecOther != nodeVec.end();
                   ++procVecOther)
            {
              // don't check itself
              if (procVecOther != procVec)
              { 
                found = 0;
                // look at each of the procs in the other vectors
                for(std::vector<std::string>::const_iterator
                    procOther = procVecOther->begin();
                    procOther != procVecOther->end();
		    ++procOther)
                {
                  // check to see if the original proc is the same as any other
                  // if it is stop looking
                  if(*proc == *procOther)
                  {
                    found = 1;
                    break;
                  } 
                }  //procOther

                // if the proc was found stop don't want it
                // else continue checking the others
                if(found == 1) {
                  break;
                }
              } //end if procVecOther != procVec

              if(found == 1) {
                  break;
              }
            } //procVecOther

            // after looking through all the procs on the other nodes
            // if found is 1 then it was found on more then one node don't want
            // if found is 0 then it was only found on 1 node save it
            if(found == 0) {
              // this proc was found on only one node so save it
              outVec.push_back(*proc);
            }
          } // proc
        } //procVec

        // output the outVec
        if (outVec.size() > 0) {
            emitOutput<diffVec>("out", outVec);
	} else {
	    outVec.push_back("");
            emitOutput<diffVec>("out", outVec);
	}
        //reset the counter
        runNum = 0;
      }
    }
}; // end class psDiff

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(psDiff)

/**
 * component for the ps command proc's
 **/
class __attribute__ ((visibility ("hidden"))) psCmd :
    public Component
{

public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(
          reinterpret_cast<Component*>(new psCmd())
        );
    }

private:
    /** Default constructor. */
    psCmd() :
        Component(Type(typeid(psCmd)), Version(1, 0, 0))
    {
        declareInput<std::string>(
            "in", boost::bind(&psCmd::inHandler, this, _1)
            );
        declareOutput<std::vector <std::string> >("out");
    }

    /** Handler for the "in" input.*/
    // Apparently the FE client sends a std::string "start"
    // to this input. Could be used to pass options to ps.
    // eg.  always perform hostname; ps and default to the
    // -e -o comm= -o euser= option for ps.  But allow overriding those.
    void inHandler(const std::string& in)
    { 
      char buffer[100];
      memset(&buffer,0,sizeof(buffer));
      FILE *p = NULL;
      std::string outline = "";
      std::vector<std::string> output;

#if 0
      int rank = Impl::TheTopologyInfo.Rank;
      std::string str = "BErank:" + boost::lexical_cast<std::string>(rank);
      output.push_back(str+"\n");
#endif

      // get ps
      p = popen("hostname; ps -e -o comm= -o euser=", "r");
      if(p != NULL) {
	while(fgets(buffer, sizeof(buffer), p) != NULL)
        {
          outline.assign(buffer);
          output.push_back(outline);
        }
        pclose(p);
      } //end if p for hostname

      emitOutput<std::vector<std::string> >("out", output );
    }
}; // end class psCmd

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(psCmd)
