#include "BrainF.h"

using namespace llvm;

int main(int argc, char *argv[])
{
  std::string s = ">++++++++[<+++++++++>-]<.>>+>+>++>[-]+<[>[->+<<++++>]<<]>.+++++++..+++.>>+++++++.<<<[[-]<[-]>]<+++++++++++++++.>>.+++.------.--------.>>+.>++++.";
  
  Parser parser(s);
  
  // Create the context and the module
  LLVMContext &C = getGlobalContext();
  ErrorOr<Module *> ModuleOrErr = new Module("my test", C);
  std::unique_ptr<Module> Owner = std::unique_ptr<Module>(ModuleOrErr.get());
  Module *M = Owner.get();

  // Create the main function: "i32 @main()"
  Function *MainF = cast<Function>(M->getOrInsertFunction("main", Type::getInt32Ty(C), (Type *)0));

  // Create the entry block
  BasicBlock *BB = BasicBlock::Create(C,
                                      "EntryBlock", // Conventionnaly called "EntryBlock"
                                      MainF // Add it to "main" function
                                      );
  IRBuilder<> B(BB); // Create a builder to add instructions
  B.SetInsertPoint(BB); // Insert the block to function

  // Generate IR code from parser
  parser.CodeGen(M, B);
  
  // Return 0 to the "main" function
  B.CreateRet(B.getInt32(0));
  
  // Print (dump) the module
  M->dump();
  
  // Default initialisation
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  
  // Create the execution engine
  std::string ErrStr;
  EngineBuilder *EB = new EngineBuilder(std::move(Owner));
  ExecutionEngine *EE = EB->setErrorStr(&ErrStr)
    .setMCJITMemoryManager(std::unique_ptr<SectionMemoryManager>(new SectionMemoryManager()))
    .create();

  if (!ErrStr.empty()) {
    std::cout << ErrStr << "\n";
    exit(0);
  }

  // Finalize the execution engine before use it
  EE->finalizeObject();

  // Run the program
  std::cout << "\n" << "=== Program Output ===" << "\n";
  std::vector<GenericValue> Args(0); // No args
  GenericValue gv = EE->runFunction(MainF, Args);
  
  // Clean up and shutdown
  delete EE;
  llvm_shutdown();
  
  return 0;
}