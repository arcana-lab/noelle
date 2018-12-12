#include "LoopEnvironment.hpp"

using namespace llvm ;

LoopEnvironment::LoopEnvironment(PDG *loopDG, SmallVector<BasicBlock *, 10> &exitBlocks) {
  for (auto nodeI : loopDG->externalNodePairs()) {
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();

    /*
     * Determine whether the external value is a producer
     */
    bool isProducer = false;
    for (auto edge : externalNode->getOutgoingEdges())
    {
      if (edge->isMemoryDependence() || edge->isControlDependence()) continue;
      isProducer = true;
      this->prodConsumers[externalValue].insert(edge->getIncomingT());
    }
    if (isProducer) this->addPreLoopProducer(externalValue);

    /*
     * Determine whether the external value is a consumer
     */
    for (auto edge : externalNode->getIncomingEdges())
    {
      if (edge->isMemoryDependence() || edge->isControlDependence()) continue;
      auto internalValue = edge->getOutgoingT();
      if (!this->isProducer(internalValue)) this->addPostLoopProducer(internalValue);
      this->prodConsumers[internalValue].insert(externalValue);
    }
  }

  this->hasExitBlockEnv = exitBlocks.size() > 1;
  if (this->hasExitBlockEnv) {
    this->exitBlockType = IntegerType::get(exitBlocks[0]->getContext(), 32);
  }
}

Type * LoopEnvironment::typeOfEnv (int index) const {
  if (index < envProducers.size()) {
    return envProducers[index]->getType();
  }

  return exitBlockType;
}
      
void LoopEnvironment::addProducer (Value *producer, bool liveIn){
  auto envIndex = envProducers.size();
  envProducers.push_back(producer);
  producerIndexMap[producer] = envIndex;
  if (liveIn) {
    liveInInds.insert(envIndex);
  } else {
    liveOutInds.insert(envIndex);
  }

  return ;
}

bool LoopEnvironment::isProducer (Value *producer) const {
  return producerIndexMap.find(producer) != producerIndexMap.end();
}

bool LoopEnvironment::isLiveIn (Value *val) {
  return isProducer(val) && liveInInds.find(producerIndexMap[val]) != liveInInds.end();
}
      
void LoopEnvironment::addPreLoopProducer (Value *producer) { 
  addProducer(producer, true); 

  return ;
}
      
void LoopEnvironment::addPostLoopProducer (Value *producer) { 
  addProducer(producer, false); 

  return ;
}
      
int LoopEnvironment::indexOfExitBlock (void) const {
  return hasExitBlockEnv ? envProducers.size() : -1; 
}
      
int LoopEnvironment::envSize (void) const {
  return envProducers.size() + (hasExitBlockEnv ? 1 : 0);
}

/*
 * DEPRECATED(angelo): See LoopEnvironment.hpp for reason
 */
int LoopEnvironment::indexOfProducer (Value *producer) {
  return producerIndexMap[producer];
}

std::set<Value *> &LoopEnvironment::consumersOf (Value *prod) {
  return prodConsumers[prod];
}
