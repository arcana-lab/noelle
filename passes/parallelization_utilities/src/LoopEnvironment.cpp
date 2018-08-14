#include "LoopEnvironment.hpp"

using namespace llvm ;

Type * LoopEnvironment::typeOfEnv (int index){
  if (index < envProducers.size()) {
    return envProducers[index]->getType();
  }

  return exitBlockType;
}
      
void LoopEnvironment::addProducer (Value *producer, bool preLoop){
  auto envIndex = envProducers.size();
  envProducers.push_back(producer);
  producerIndexMap[producer] = envIndex;
  if (preLoop) {
    preLoopEnv.insert(envIndex);
  } else {
    postLoopEnv.insert(envIndex);
  }

  return ;
}
      
bool LoopEnvironment::isPreLoopEnv (Value *val){
  return producerIndexMap.find(val) != producerIndexMap.end() && preLoopEnv.find(producerIndexMap[val]) != preLoopEnv.end();
}
      
void LoopEnvironment::addPreLoopProducer (Value *producer) { 
  addProducer(producer, true); 

  return ;
}
      
void LoopEnvironment::addPostLoopProducer (Value *producer) { 
  addProducer(producer, false); 

  return ;
}
      
int LoopEnvironment::indexOfExitBlock (void) { 
  return envProducers.size(); 
}
      
int LoopEnvironment::envSize (void) { 
  return envProducers.size() + 1; 
}
