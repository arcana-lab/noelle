FetchContent_Declare(
  leptoinst
  GIT_REPOSITORY  "https://github.com/fsossai/lepto-inst.git"
)
FetchContent_GetProperties(leptoinst)

if(NOT rapidjson_POPULATED)
  FetchContent_Populate(rapidjson)
endif()

if(NOT lepto_POPULATED)
  FetchContent_Populate(leptoinst)
endif()

include_directories(${leptoinst_SOURCE_DIR}/include)

noelle_tool_declare(LDGDot)
target_sources(
  LDGDot
  PRIVATE
  src/LDGDot.cpp
  src/Pass.cpp
  ${leptoinst_SOURCE_DIR}/src/LeptoInstVisitor.cpp
)

