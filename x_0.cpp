#include "souffle/CompiledSouffle.h"

namespace souffle {
using namespace ram;
class Sf_x_0 : public SouffleProgram {
private:
static inline bool regex_wrapper(const char *pattern, const char *text) {
   bool result = false; 
   try { result = std::regex_match(text, std::regex(pattern)); } catch(...) { 
     std::cerr << "warning: wrong pattern provided for match(\"" << pattern << "\",\"" << text << "\")\n";
}
   return result;
}
static inline std::string substr_wrapper(const char *str, size_t idx, size_t len) {
   std::string sub_str, result; 
   try { result = std::string(str).substr(idx,len); } catch(...) { 
     std::cerr << "warning: wrong index position provided by substr(\"";
     std::cerr << str << "\"," << idx << "," << len << ") functor.\n";
   } return result;
}
public:
SymbolTable symTable;
// -- Table: A
ram::Relation<Auto,1>* rel_1_A;
souffle::RelationWrapper<0,ram::Relation<Auto,1>,Tuple<RamDomain,1>,1,true,false> wrapper_rel_1_A;
// -- Table: r_bxor
ram::Relation<Auto,3>* rel_2_r_bxor;
souffle::RelationWrapper<1,ram::Relation<Auto,3>,Tuple<RamDomain,3>,3,false,true> wrapper_rel_2_r_bxor;
public:
Sf_x_0() : rel_1_A(new ram::Relation<Auto,1>()),
wrapper_rel_1_A(*rel_1_A,symTable,"A",std::array<const char *,1>{"i:number"},std::array<const char *,1>{"x"}),
rel_2_r_bxor(new ram::Relation<Auto,3>()),
wrapper_rel_2_r_bxor(*rel_2_r_bxor,symTable,"r_bxor",std::array<const char *,3>{"i:number","i:number","i:number"},std::array<const char *,3>{"x","y","z"}){
addRelation("A",&wrapper_rel_1_A,1,0);
addRelation("r_bxor",&wrapper_rel_2_r_bxor,0,1);
}
~Sf_x_0() {
delete rel_1_A;
delete rel_2_r_bxor;
}
private:
template <bool performIO> void runFunction(std::string inputDirectory = ".", std::string outputDirectory = ".") {
// -- initialize counter --
std::atomic<RamDomain> ctr(0);

#if defined(__EMBEDDED_SOUFFLE__) && defined(_OPENMP)
omp_set_num_threads(1);
#endif

// -- query evaluation --
/* BEGIN visitSequence @RamExecutor.cpp:1432 */
/* BEGIN visitCreate @RamExecutor.cpp:1230 */
/* END visitCreate @RamExecutor.cpp:1231 */
/* BEGIN visitLoad @RamExecutor.cpp:1242 */
if (performIO) {
try {std::map<std::string, std::string> directiveMap({{"IO","file"},{"delimiter","\t"},{"filename","/home/lyndonhenry/Dropbox/workspace/souffle/tests/testsuite.dir/semantic/bitwise/id/A.facts"},{"intermediate","true"},{"name","A"}});
if (!inputDirectory.empty() && directiveMap["IO"] == "file" && directiveMap["filename"].front() != '/') {directiveMap["filename"] = inputDirectory + "/" + directiveMap["filename"];}
IODirectives ioDirectives(directiveMap);
IOSystem::getInstance().getReader(SymbolMask({0}), symTable, ioDirectives, 0)->readAll(*rel_1_A);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
/* END visitLoad @RamExecutor.cpp:1261 */
/* BEGIN visitCreate @RamExecutor.cpp:1230 */
/* END visitCreate @RamExecutor.cpp:1231 */
/* BEGIN visitDebugInfo @RamExecutor.cpp:1516 */
SignalHandler::instance()->setMsg(R"_(r_bxor((xbxory),x,y) :- 
   A(x),
   A(y).
in file /home/lyndonhenry/Dropbox/workspace/souffle/tests/semantic/bitwise/bitwise.dl [13:1-13:38])_");
/* BEGIN visitInsert @RamExecutor.cpp:1287 */
if (!rel_1_A->empty()) {
auto part = rel_1_A->partition();
PARALLEL_START;
CREATE_OP_CONTEXT(rel_1_A_op_ctxt,rel_1_A->createContext());
CREATE_OP_CONTEXT(rel_2_r_bxor_op_ctxt,rel_2_r_bxor->createContext());
/* BEGIN visitScan @RamExecutor.cpp:1543 */
pfor(auto it = part.begin(); it<part.end(); ++it) 
try{for(const auto& env0 : *it) {
/* BEGIN visitSearch @RamExecutor.cpp:1529 */
/* BEGIN visitScan @RamExecutor.cpp:1543 */
for(const auto& env1 : *rel_1_A) {
/* BEGIN visitSearch @RamExecutor.cpp:1529 */
/* BEGIN visitProject @RamExecutor.cpp:1783 */
Tuple<RamDomain,3> tuple({(RamDomain)(/* BEGIN visitBinaryOperator @RamExecutor.cpp:2039 */
(/* BEGIN visitElementAccess @RamExecutor.cpp:1960 */
env0[0]/* END visitElementAccess @RamExecutor.cpp:1962 */
) ^ (/* BEGIN visitElementAccess @RamExecutor.cpp:1960 */
env1[0]/* END visitElementAccess @RamExecutor.cpp:1962 */
)/* END visitBinaryOperator @RamExecutor.cpp:2111 */
),(RamDomain)(/* BEGIN visitElementAccess @RamExecutor.cpp:1960 */
env0[0]/* END visitElementAccess @RamExecutor.cpp:1962 */
),(RamDomain)(/* BEGIN visitElementAccess @RamExecutor.cpp:1960 */
env1[0]/* END visitElementAccess @RamExecutor.cpp:1962 */
)});
rel_2_r_bxor->insert(tuple,READ_OP_CONTEXT(rel_2_r_bxor_op_ctxt));
/* END visitProject @RamExecutor.cpp:1838 */
/* END visitSearch @RamExecutor.cpp:1539 */
}
/* END visitSearch @RamExecutor.cpp:1539 */
}
} catch(std::exception &e) { SignalHandler::instance()->error(e.what());}
PARALLEL_END;
}
/* END visitInsert @RamExecutor.cpp:1378 */
/* END visitDebugInfo @RamExecutor.cpp:1523 */
/* BEGIN visitDrop @RamExecutor.cpp:1397 */
if (performIO || 0) rel_1_A->purge();
/* END visitDrop @RamExecutor.cpp:1401 */
/* BEGIN visitStore @RamExecutor.cpp:1265 */
if (performIO) {
try {std::map<std::string, std::string> directiveMap({{"IO","file"},{"filename","/home/lyndonhenry/Dropbox/workspace/souffle/tests/testsuite.dir/semantic/bitwise/id/r_bxor.csv"},{"name","r_bxor"}});
if (!outputDirectory.empty() && directiveMap["IO"] == "file" && directiveMap["filename"].front() != '/') {directiveMap["filename"] = outputDirectory + "/" + directiveMap["filename"];}
IODirectives ioDirectives(directiveMap);
IOSystem::getInstance().getWriter(SymbolMask({0, 0, 0}), symTable, ioDirectives, 0)->writeAll(*rel_2_r_bxor);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
/* END visitStore @RamExecutor.cpp:1283 */
/* BEGIN visitDrop @RamExecutor.cpp:1397 */
if (performIO || 0) rel_2_r_bxor->purge();
/* END visitDrop @RamExecutor.cpp:1401 */
/* END visitSequence @RamExecutor.cpp:1436 */

// -- relation hint statistics --
if(isHintsProfilingEnabled()) {
std::cout << " -- Operation Hint Statistics --\n";
std::cout << "Relation rel_1_A:\n";
rel_1_A->printHintStatistics(std::cout,"  ");
std::cout << "\n";
std::cout << "Relation rel_2_r_bxor:\n";
rel_2_r_bxor->printHintStatistics(std::cout,"  ");
std::cout << "\n";
}
}
public:
void run() { runFunction<false>(); }
public:
void runAll(std::string inputDirectory = ".", std::string outputDirectory = ".") { runFunction<true>(inputDirectory, outputDirectory); }
public:
void printAll(std::string outputDirectory = ".") {
try {std::map<std::string, std::string> directiveMap({{"IO","file"},{"filename","/home/lyndonhenry/Dropbox/workspace/souffle/tests/testsuite.dir/semantic/bitwise/id/r_bxor.csv"},{"name","r_bxor"}});
if (!outputDirectory.empty() && directiveMap["IO"] == "file" && directiveMap["filename"].front() != '/') {directiveMap["filename"] = outputDirectory + "/" + directiveMap["filename"];}
IODirectives ioDirectives(directiveMap);
IOSystem::getInstance().getWriter(SymbolMask({0, 0, 0}), symTable, ioDirectives, 0)->writeAll(*rel_2_r_bxor);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
public:
void loadAll(std::string inputDirectory = ".") {
try {std::map<std::string, std::string> directiveMap({{"IO","file"},{"delimiter","\t"},{"filename","/home/lyndonhenry/Dropbox/workspace/souffle/tests/testsuite.dir/semantic/bitwise/id/A.facts"},{"intermediate","true"},{"name","A"}});
if (!inputDirectory.empty() && directiveMap["IO"] == "file" && directiveMap["filename"].front() != '/') {directiveMap["filename"] = inputDirectory + "/" + directiveMap["filename"];}
IODirectives ioDirectives(directiveMap);
IOSystem::getInstance().getReader(SymbolMask({0}), symTable, ioDirectives, 0)->readAll(*rel_1_A);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
public:
void dumpInputs(std::ostream& out = std::cout) {
try {IODirectives ioDirectives;
ioDirectives.setIOType("stdout");
ioDirectives.setRelationName("rel_1_A");
IOSystem::getInstance().getWriter(SymbolMask({0}), symTable, ioDirectives, 0)->writeAll(*rel_1_A);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
public:
void dumpOutputs(std::ostream& out = std::cout) {
try {IODirectives ioDirectives;
ioDirectives.setIOType("stdout");
ioDirectives.setRelationName("rel_2_r_bxor");
IOSystem::getInstance().getWriter(SymbolMask({0, 0, 0}), symTable, ioDirectives, 0)->writeAll(*rel_2_r_bxor);
} catch (std::exception& e) {std::cerr << e.what();exit(1);}
}
public:
const SymbolTable &getSymbolTable() const {
return symTable;
}
};
SouffleProgram *newInstance_x(){return new Sf_x_0;}
SymbolTable *getST_x(SouffleProgram *p){return &reinterpret_cast<Sf_x_0*>(p)->symTable;}
#ifdef __EMBEDDED_SOUFFLE__
class factory_Sf_x_0: public souffle::ProgramFactory {
SouffleProgram *newInstance() {
return new Sf_x_0();
};
public:
factory_Sf_x_0() : ProgramFactory("x"){}
};
static factory_Sf_x_0 __factory_Sf_x_0_instance;
}
#else
}
int main(int argc, char** argv)
{
try{
souffle::CmdOptions opt(R"(/home/lyndonhenry/Dropbox/workspace/souffle/tests/semantic/bitwise/bitwise.dl)",
R"(.)",
R"(.)",
false,
R"()",
1
);
if (!opt.parse(argc,argv)) return 1;
#if defined(_OPENMP) 
omp_set_nested(true);
#endif
souffle::Sf_x_0 obj;
obj.runAll(opt.getInputFileDir(), opt.getOutputFileDir());
return 0;
} catch(std::exception &e) { souffle::SignalHandler::instance()->error(e.what());}
}
#endif
