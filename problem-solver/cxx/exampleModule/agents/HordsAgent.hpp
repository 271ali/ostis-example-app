#pragma once
#include <sc-memory/kpm/sc_agent.hpp>
#include "keynodes/keynodes.hpp"
#include "HordsAgent.generated.hpp"
#include <vector>
using namespace std;
namespace exampleModule
{
  struct Hord
  {
    ScAddr node1;
    ScAddr node2;
    ScAddr edge;
  };
  using Cycle=std::vector<ScAddr>;
  using VisitedNodes=std::vector<ScAddr>;
  using Hords=std::vector<Hord>;
  class HordsAgent:public ScAgent{
     SC_CLASS(Agent, Event(Keynodes::question_find_hords, ScEvent::Type::AddOutputEdge))
     SC_GENERATED_BODY()
     void answerConstruction( ScAddr const & inputGraphStruct,ScAddrVector answerElements,Hords hords);
     bool noOrientCheck(ScAddr const & inputGraphStruct);
     vector<Cycle> findCycles(ScAddr const & inputGraphStruct);
     void dfs(ScAddr startNode,ScAddr currentNode,VisitedNodes vn,vector<Cycle> result,Cycle cycle);
     Hords getHords(vector<Cycle> cycles);
  };
 
}
