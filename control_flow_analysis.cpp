#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <list>
#include <fstream>

using namespace llvm;
using namespace std;

class ControlFG : public ModulePass {
public:
    static char ID;
    ControlFG() : ModulePass(ID) {}
    bool runOnModule(Module &M) {

    	string filename = getFileName(M, false);
    	errs() << "Control Flow Analysis: " << filename << " \n\n";

        
        controlflowfile.open(filename + "_controlflow.txt");

    	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; F++) {
            createFlowGraph(*F, true);
        }
        controlflowfile.close();

    	return false;
    }
private:
	/* data */
    ofstream controlflowfile;//control flow graph
    map<Type *, vector<Function *>> FunctionList; 
    string currentfilename;

	/* main fuction */
    string getFileName(Module &M, bool isPrint);
	void createFlowGraph(Function &F, bool isPrint);
    void helpCalledFunction(Function *called, unsigned linenumber, bool isPrint);

    void updateFunctionList(Type *t, Function *F){
        if(FunctionList.find(t) == FunctionList.end()){ //更新FunctionList
            vector<Function *> v;
            v.push_back(F);
            FunctionList.insert(map<Type *, vector<Function *>>::value_type(t,v));
        }
        else{
            vector<Function *> v = FunctionList.find(t)->second;
            if(find(v.begin(),v.end(),F) == v.end())
                (FunctionList.find(t)->second).push_back(F);
        }
    }
    vector<Function *> getFuncList(Type *t){
        Type *ft = t->getPointerElementType();
        vector<Function *> funclist;
        funclist = FunctionList[ft];

        // for(vector<Function *>::iterator i = funclist.begin(), e = funclist.end(); i != e; i++){
        //     errs() << "function: " << (*i)->getName().str() << "\n";
        // }
        return funclist;
    }
};

/////////////////////////////////////////////////////////////////////////
//                            main functions                           //
/////////////////////////////////////////////////////////////////////////
string ControlFG::getFileName(Module &M, bool isPrint){
    string filename = M.getName().str();
    filename = filename.substr(0,filename.find(".bc"));
    if(isPrint)
        errs() << "filename: " << filename << "\n";

    return filename;
}

void ControlFG::createFlowGraph(Function &F, bool isPrint) {
    Type *t = F.getFunctionType();
    Function *Func = &F;
    updateFunctionList(t, Func);

    bool funcflag = true;
    unsigned funcLinenumber;
    if(F.hasMetadata()){
        SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
        F.getAllMetadata(MDs);
        for (auto &MD : MDs) {
          if (MDNode *N = MD.second) {
            if (auto *subProgram = dyn_cast<DISubprogram>(N)) {
              //errs() << subProgram->getLine() << "\n";
                funcLinenumber = subProgram->getLine();
                currentfilename = subProgram->getFilename().str();
                controlflowfile << "\n" << "%function name:" << subProgram->getName().str() << "\n";
                if(isPrint)
                    errs() << "\n" << "%function name:" << subProgram->getName() << "\n";
            }
          }
        }
    }
    if(F.getName().str() == "main"){
        controlflowfile << "start," << currentfilename << ":" << funcLinenumber << "\n";
        if(isPrint)
            errs() << "start," << currentfilename << ":" << funcLinenumber << "\n";
    }
	for (Function::iterator B = F.begin(), BE = F.end(); B != BE; B++) {
        for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; I++) {
            // if(isPrint){
            //     I->dump();
            // }
            unsigned linenumber; //current line number if any

            /*get control flow*/
            if (DILocation *Loc = I->getDebugLoc()) { // Here I is an LLVM instruction
                linenumber = Loc->getLine();
                if(F.arg_empty() && (linenumber != funcLinenumber) && funcflag){
                    controlflowfile << currentfilename << ":" << funcLinenumber << "," << currentfilename << ":" << linenumber << "\n";
                    if(isPrint)
                        errs() << currentfilename << ":" << funcLinenumber << "," << currentfilename << ":" << linenumber << "\n";
                    funcflag = false;
                }
                
                if(BranchInst *BI = dyn_cast<BranchInst>(I)){
                    for(unsigned i = 0; i < BI->getNumSuccessors(); i++){
                        BasicBlock *succ = BI->getSuccessor(i);
                        Instruction &front =  succ->front();
                        Instruction *next = &front;

                        while(isa<BranchInst>(next) && !next->getDebugLoc()){
                            succ = dyn_cast<BranchInst>(next)->getSuccessor(0);
                            next = &(succ->front());
                        }

                        while(!next->getDebugLoc()){
                            next->dump();
                            next = next->getNextNode();
                        }
                        unsigned nextlinenumber = next->getDebugLoc()->getLine();
                        if(linenumber != nextlinenumber){
                            controlflowfile << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                            if(isPrint)
                                errs()  << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                        }

                    }
                }else if(SwitchInst *SI = dyn_cast<SwitchInst>(I)){
                    for(unsigned i = 0; i < SI->getNumSuccessors(); i++){
                        BasicBlock *succ = SI->getSuccessor(i);
                        Instruction &front =  succ->front();
                        Instruction *next = &front;
                        while(!next->getDebugLoc())
                            next = next->getNextNode();
                        unsigned nextlinenumber = next->getDebugLoc()->getLine();
                        if(linenumber != nextlinenumber){
                            controlflowfile << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                            if(isPrint)
                                errs() << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                        }
                    }
                }else{
                    if(Instruction *next = I->getNextNode()){
                        BasicBlock *succ;
                        while(isa<BranchInst>(next) && !next->getDebugLoc()){
                            succ = dyn_cast<BranchInst>(next)->getSuccessor(0);
                            next = &(succ->front());
                        }

                        while(!next->getDebugLoc())
                            next = next->getNextNode();
                        unsigned nextlinenumber = next->getDebugLoc()->getLine();
                        if(linenumber != nextlinenumber){
                            controlflowfile << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                            if(isPrint)
                                errs() << currentfilename << ":" <<  linenumber << "," << currentfilename << ":" <<  nextlinenumber << "\n";
                        }
                    }
                    if(StoreInst *SI = dyn_cast<StoreInst>(I)){
                        Value *v = SI->getValueOperand();
                        if(ConstantExpr *CE = dyn_cast<ConstantExpr>(v)){
                            if(CE->getOpcode() == Instruction::BitCast){
                                Type *t = CE->getType();
                                v = CE->getOperand(0);
                                if(Function *F = dyn_cast<Function>(v)){
                                    updateFunctionList(t->getPointerElementType(),F);
                                }
                            }
                        }
                    }

                    if(CallInst *CI = dyn_cast<CallInst>(I)){
                        Function *called = CI->getCalledFunction();
                        string calledFuncName;

                        if(called){

                            if(called->getName().str() == "llvm.dbg.declare")
                                continue;

                            controlflowfile << "%direct call to: " << called->getName().str() << "\n";
                            if(isPrint)
                                errs() << "%direct call to: " << called->getName() << "\n";

                            if(!called->hasMetadata()){
                                controlflowfile << currentfilename << ":" <<  linenumber << "," << called->getName().str() << "\n";
                                if(isPrint)
                                    errs() << currentfilename << ":" <<  linenumber << "," << called->getName() << "\n";
                                
                                controlflowfile << called->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";
                                if(isPrint)
                                    errs() << called->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";

                                continue;
                            }

                            helpCalledFunction(called, linenumber, isPrint);

                            
                        }
                        else if(CI->getNumOperands() > 1){
                            if(ConstantExpr *CstExpr = dyn_cast<ConstantExpr>(CI->getOperand(1))){
                                if (CstExpr->isCast()) {
                                    Function *castFunc = dyn_cast<Function>(CstExpr->getOperand(0));

                                    controlflowfile << "%direct call to: " << castFunc->getName().str() << "\n";
                                    if(isPrint)
                                        errs() << "%direct call to: " << castFunc->getName() << "\n";
                                    if(!castFunc->hasMetadata()){
                                        controlflowfile << currentfilename << ":" <<  linenumber << "," << castFunc->getName().str() << "\n";
                                        if(isPrint)
                                            errs() << currentfilename << ":" <<  linenumber << "," << castFunc->getName() << "\n";
                                        
                                        controlflowfile << castFunc->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";
                                        if(isPrint)
                                            errs() << castFunc->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";

                                        continue;
                                    }

                                    helpCalledFunction(castFunc, linenumber, isPrint);

                                }
                            }else{
                                Type *t = CI->getCalledValue()->getType();
                                vector<Function *> funclist = getFuncList(t);
                                for(vector<Function *>::iterator i = funclist.begin(), e = funclist.end(); i != e; i++){
                                    Function *indirectcalled = dyn_cast<Function>(*i);
                                    calledFuncName = indirectcalled->getName().str();
                                    controlflowfile << "%indirect call may to: " << calledFuncName << "\n";
                                    if(isPrint)
                                        errs() << "%indirect call may to: " << calledFuncName << "\n";

                                    if(!indirectcalled->hasMetadata()){
                                        controlflowfile << currentfilename << ":" <<  linenumber << "," << indirectcalled->getName().str() << "\n";
                                        if(isPrint)
                                            errs() << currentfilename << ":" <<  linenumber << "," << indirectcalled->getName() << "\n";
                                        
                                        controlflowfile << indirectcalled->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";
                                        if(isPrint)
                                            errs() << indirectcalled->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";

                                        continue;
                                    }

                                    helpCalledFunction(indirectcalled, linenumber, isPrint);
                                }
                            }
                        }else{
                            Type *t = CI->getCalledValue()->getType();
                            vector<Function *> funclist = getFuncList(t);
                            for(vector<Function *>::iterator i = funclist.begin(), e = funclist.end(); i != e; i++){
                                Function *indirectcalled = dyn_cast<Function>(*i);
                                calledFuncName = indirectcalled->getName().str();
                                controlflowfile << "%indirect call may to: " << calledFuncName << "\n";
                                if(isPrint)
                                    errs() << "%indirect call may to: " << calledFuncName << "\n";

                                if(!indirectcalled->hasMetadata()){
                                    controlflowfile << currentfilename << ":" <<  linenumber << "," << indirectcalled->getName().str() << "\n";
                                    if(isPrint)
                                        errs() << currentfilename << ":" <<  linenumber << "," << indirectcalled->getName() << "\n";
                                    
                                    controlflowfile << indirectcalled->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";
                                    if(isPrint)
                                        errs() << indirectcalled->getName().str() << "," << currentfilename << ":" <<  linenumber << "\n";

                                    continue;
                                }

                                helpCalledFunction(indirectcalled, linenumber, isPrint);
                            }
                        }

                    }
                }
                
            }        

        }
         
    }
}

void ControlFG::helpCalledFunction(Function *called, unsigned linenumber, bool isPrint) {
    string calledfilename;

    SmallVector<std::pair<unsigned, MDNode *>, 4> newMDs;
    called->getAllMetadata(newMDs);
    for (auto &newMD : newMDs) {
      if (MDNode *newN = newMD.second) {
        if (auto *newsubProgram = dyn_cast<DISubprogram>(newN)) {
            calledfilename = newsubProgram->getFilename().str();    
        }
      }
    }

    unsigned nextlinenumber;
    if(called->hasMetadata()){
        SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
        called->getAllMetadata(MDs);
        for (auto &MD : MDs) {
          if (MDNode *N = MD.second) {
            if (auto *subProgram = dyn_cast<DISubprogram>(N)) {
              //errs() << subProgram->getLine() << "\n";
                nextlinenumber = subProgram->getLine();
            }
          }
        }
    }

    //flow to function head
    // BasicBlock &Bfront = called->front();
    // Instruction &front =  Bfront.front();
    
    // while(!next->getDebugLoc())
    //     next = next->getNextNode();
    // unsigned nextlinenumber = next->getDebugLoc()->getLine();
    if(linenumber != nextlinenumber){
        controlflowfile << currentfilename << ":" <<  linenumber << "," << calledfilename << ":" << nextlinenumber << "\n";
        if(isPrint)
            errs() << currentfilename << ":" <<  linenumber << "," << calledfilename << ":" << nextlinenumber << "\n";
    }

    //flow back from function end
    BasicBlock &Bback = called->back();
    Instruction &back =  Bback.back();
    Instruction *prev = &back;
    while(!prev->getDebugLoc())
        prev = prev->getPrevNode();
    unsigned prevlinenumber = prev->getDebugLoc()->getLine();
    if(linenumber != prevlinenumber){
        controlflowfile << calledfilename << ":" << prevlinenumber << "," << currentfilename << ":" << linenumber << "\n";
        if(isPrint)
            errs() << calledfilename << ":" << prevlinenumber << "," << currentfilename << ":" << linenumber << "\n";
    }
}

/////////////////////////////////////////////////////////////////////////
//                             install pass                            //
/////////////////////////////////////////////////////////////////////////
char ControlFG::ID = 0;
static RegisterPass<ControlFG> X("cfg", "Control Flow Analysis"
                            " pass",
                            false /* Only looks at CFG */,
                            false /* Analysis Pass */);
