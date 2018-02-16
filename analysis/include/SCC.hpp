#pragma once

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {

	/*
	* Strongly Connected Component
	*/
	template <class T>
	class SCC {
	 public:
		SCC(std::vector<T *> comp) : components(comp) {}
		~SCC() {}

   private:
   	std::vector<T *> components;
	};
}