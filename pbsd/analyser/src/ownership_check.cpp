// PBSD Static Analyser — Layer 2 Ownership Dataflow
// Author: Odin Loch
// PROVENANCE: Original implementation using Clang LibTooling public API.
//
// This file is the skeleton for the custom borrow-checker equivalent.
// It uses Clang's AST and CFG infrastructure to track state transitions
// of UniqueHandle / BorrowedHandle / SharedHandle across the control-flow
// graph of each function, flagging:
//   - Use-after-move
//   - Double-release
//   - Exclusive-borrow-while-aliased
//
// Layer 1 (syntactic bans) and Layer 3 (Clang Thread Safety Analysis) are
// enabled via compiler flags in CMakeLists.txt — no source needed here.
//
// TODO: Implement dataflow analysis. Start with a clang-tidy check
// banning raw new/delete as Layer 1 proof-of-concept.

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

namespace {

class YggOwnershipConsumer : public ASTConsumer {
public:
    explicit YggOwnershipConsumer(ASTContext& /*ctx*/) {}
    // TODO: traverse declarations and build ownership dataflow per function
};

class YggOwnershipAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<YggOwnershipConsumer>(CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance&,
                   const std::vector<std::string>&) override { return true; }
};

} // namespace

static FrontendPluginRegistry::Add<YggOwnershipAction>
    X("pbsd-ownership", "PBSD ownership/lifetime checker");
