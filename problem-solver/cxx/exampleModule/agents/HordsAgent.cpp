#include <iostream>
#include <sc-agents-common/utils/GenerationUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>
#include <sc-agents-common/utils/IteratorUtils.hpp>
#include "HordsAgent.hpp"

using namespace std;
using namespace utils;

namespace exampleModule
{
    
SC_AGENT_IMPLEMENTATION(HordsAgent)
{
  SC_LOG_DEBUG("BEGINNNN");
  SC_LOG_ERROR("BEGIN");
  ScAddr actionNode = otherAddr;
  SC_LOG_ERROR("actionNode is done");
  ScAddr inputGraphStruct = IteratorUtils::getAnyFromSet(ms_context.get(), actionNode);
  SC_LOG_ERROR("inputGraph is done");
  if(!inputGraphStruct.IsValid())
  {
  SC_LOG_ERROR("HordsAgent: Invalid argument");
  utils::AgentUtils::finishAgentWork(&m_memoryCtx,actionNode,false);
  return SC_RESULT_ERROR_INVALID_PARAMS;
  }
  if (noOrientCheck(inputGraphStruct)==false)
  {
    SC_LOG_ERROR("Graph is oriented");
    utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
   vector<Cycle> cycles=findCycles(inputGraphStruct);
   if (cycles.empty())
  {
    SC_LOG_ERROR("No cycles in graph");
    utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
    Hords hords=getHords(cycles); 
     if (hords.empty())
  {
    SC_LOG_ERROR("No hords in graph");
    utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, false);
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
  ScAddrVector answerElements;
  answerConstruction(inputGraphStruct,answerElements,hords);
  utils::AgentUtils::finishAgentWork(&m_memoryCtx, actionNode, answerElements, true);
  SC_LOG_DEBUG("succesfully");
  return SC_RESULT_OK;
}

void HordsAgent::answerConstruction( ScAddr const & inputGraphStruct,ScAddrVector answerElements,Hords hords)
{ 
    ScAddr tupleNode=m_memoryCtx.CreateNode(ScType::NodeConstTuple);
    for(auto hord:hords)
    {
       ScAddr hordNode=m_memoryCtx.CreateNode(ScType::NodeConst);
       ScAddr hordEdge1=m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::HordsClass,hordNode);
       ScAddr hordEdge2=m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm,hordNode,hord.node1);
       ScAddr hordEdge3=m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm,hordNode,hord.edge);
       ScAddr hordEdge4=m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm,hordNode,hord.node2);
       ScAddr hordEdge5=m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm,tupleNode,hordNode);
       answerElements.insert(answerElements.end(),{hordNode,hordEdge1,hordEdge2,hordEdge3,hordEdge4,hordEdge5});
       
   }
    ScAddr graphAndHordsEdge = m_memoryCtx.CreateEdge(ScType::EdgeDCommonConst, inputGraphStruct,tupleNode);
    ScAddr relation = m_memoryCtx.CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::nrel_hords, graphAndHordsEdge);
    answerElements.insert(answerElements.end(),{Keynodes::HordsClass,Keynodes::nrel_hords,tupleNode,graphAndHordsEdge,relation});
}
bool HordsAgent::noOrientCheck(ScAddr const & inputGraphStruct)
{

    SC_LOG_ERROR("Check is beginning");
    ScAddrVector graphNodes;
    ScIterator3Ptr it3_0 = m_memoryCtx.Iterator3(
    inputGraphStruct,
    ScType::EdgeAccessConstPosPerm,
    ScType::NodeConst);
    while (it3_0->Next())
    {
        graphNodes.push_back( it3_0->Get(2) );
    }
    SC_LOG_ERROR("GraphNodes are defined");
    for(auto node:graphNodes)
    {
            ScAddrVector neighbourNodes;
            ScIterator3Ptr it3_1=m_memoryCtx.Iterator3(
            node,
            ScType::EdgeAccessConstPosPerm,
            ScType::NodeConst);
          while(it3_1->Next())
          {
            neighbourNodes.push_back(it3_1->Get(2));
          }
           ScIterator3Ptr it3_2=m_memoryCtx.Iterator3(
            ScType::NodeConst,
            ScType::EdgeAccessConstPosPerm,
            node);
             while (it3_2->Next())
             {
                 bool id=0;
                for(int i=0;i<neighbourNodes.size();i++)
                {
                   if(neighbourNodes[i] == it3_2->Get(0)) 
                   {
                   id=1;
                   break;
                   }
                }
                if(id==0) return false;
             }
    }
     SC_LOG_ERROR("Chek is completing");
    return true;
   
}
vector<Cycle> HordsAgent::findCycles(ScAddr const & inputGraphStruct)
{
    SC_LOG_ERROR("Start cycles finding");
    vector<Cycle> result;
    ScAddrVector graphNodes;
    ScIterator3Ptr it3=m_memoryCtx.Iterator3(
    inputGraphStruct,
    ScType::EdgeAccessConstPosPerm,
    ScType::NodeConst);
     while (it3->Next())
    { 
       graphNodes.push_back( it3->Get(2) );
    }
    SC_LOG_ERROR("GraphNodes are defined");
    for (auto node : graphNodes)
    {
        Cycle cycle;
        ScAddrVector vn;
        vector<Cycle> cycles;
        dfs(node,node,vn,cycles,cycle);
        result.insert(result.end(),cycles.begin(),cycles.end());
    }
    SC_LOG_ERROR("Finish cycles finding");
    return result;
}

void HordsAgent::dfs(ScAddr startNode,ScAddr currentNode,ScAddrVector vn,vector<Cycle> result,Cycle cycle)
{
    SC_LOG_ERROR("DFS");
    if(startNode==currentNode && !vn.empty())
    {
       result.push_back(cycle);
       return;
    }
   auto iter=find(vn.begin(),vn.end(),currentNode);
   if(iter!=vn.end()) return;
   vn.push_back(currentNode);
   cycle.push_back(currentNode);
   ScAddrVector currentNeighbourNodes;
   ScIterator3Ptr it3=m_memoryCtx.Iterator3(
   currentNode,
   ScType::EdgeAccessConstPosPerm,
   ScType::NodeConst);
   while (it3->Next())
    {
      currentNeighbourNodes.push_back( it3->Get(2) );
    }
    for(auto node:currentNeighbourNodes)
    {
        if(cycle.size()>1 && cycle[cycle.size()-2]==node) continue;
        dfs(startNode,node,vn,result,cycle);
    }
}

Hords HordsAgent::getHords(vector<Cycle> cycles)
{
    Hords result;
    SC_LOG_ERROR("Start hords finding");
    for(auto cycle:cycles)
    {
        if(cycle.size()>3)
        {
            int lastStartIndex = cycle.size() / 2 + cycles.size() % 2;
             SC_LOG_ERROR("Cycle exploring");
            for(int i=0;i<lastStartIndex;i++){
                ScIterator3Ptr it3= m_memoryCtx.Iterator3(
                cycle[i],
                ScType::EdgeAccessConstPosPerm,
                ScType::NodeConst);
                while (it3->Next())
                {
                 if(it3->Get(2)!=cycle[i+1] && it3->Get(2)!=cycle[i-1])
                 {
                    Hord hord;
                    hord.node1=cycle[i];
                    hord.node2=it3->Get(2);
                    hord.edge=it3->Get(1);
                    bool exist=0;
                    for(auto example:result)
                    {
                      if((example.node1==hord.node1&&example.node2==hord.node2)||(example.node1==hord.node2&&example.node2==hord.node1)) 
                      {
                        exist=1;
                        break;
                      }
                    }
                    if(exist==0) result.push_back(hord);
                 }
                }
            }
        }
        else continue; 
    }
}

}
