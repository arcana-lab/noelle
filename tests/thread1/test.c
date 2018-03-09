#include <iostream>
#include <vector>

#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"

int stage0(MARC::ThreadSafeQueue<int> *queue){
  int s = 13;
  for (int i = 0; i < 5; ++i) {
    s += 4;
    queue->push(s);
  }
  return s;
}

int stage1(MARC::ThreadSafeQueue<int> *queue){
  int t = 17;
  int u;
  for (int i = 0; i < 5; ++i) {
    queue->waitPop(u);
    t += u;
  }
  return t;
}

int main (int argc, char *argv[]){

  /*
   * Create a thread pool with 2 threads
   */
  MARC::ThreadPool pool(2);

  /*
   * Submit and detach the two jobs
   */
  MARC::ThreadSafeQueue<int> queue;
  MARC::ThreadSafeQueue<int> *queueP = &queue; 
  auto s = pool.submit(stage0, queueP).get();
  auto t = pool.submit(stage1, queueP).get();

  std::cout << "S: " << s << ", T: " << t << "\n"; 
  return 0;
}
