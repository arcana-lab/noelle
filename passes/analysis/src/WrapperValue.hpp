#pragma once

#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace llvm {
	class WrapperValue {
	 public:
	 	WrapperValue(Value *val) : value{val} {};
	 	WrapperValue(Instruction *inst) : value{cast<Value>(inst)} {};
	 	WrapperValue(Argument *arg) : value{cast<Value>(arg)} {};

	 	Value *getValue() { return value; }
		raw_ostream & print(raw_ostream &stream);
	 private:
		Value *value;
	};

	raw_ostream & WrapperValue::print(raw_ostream &stream)
	{
		if (value == nullptr) return (stream << "NULL value in WrapperValue");
		value->print(stream);
		return stream;
	}
}