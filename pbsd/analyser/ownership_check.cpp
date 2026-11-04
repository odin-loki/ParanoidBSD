// PBSD Static Analyser — Layer 2 Ownership Dataflow (Wave 1)
// PROVENANCE: Clang LibTooling public API.

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

constexpr llvm::StringRef kHandleSuffixes[] = {
    "UniqueHandle", "BorrowedHandle", "SharedHandle", "VmHandle",
};

bool isPbsdHandleType(QualType qt) {
    if (qt.isNull()) {
        return false;
    }
    const auto* rec = qt.getCanonicalType()->getAsRecordDecl();
    if (!rec || !rec->getIdentifier()) {
        return false;
    }
    const auto name = rec->getName();
    for (const auto suffix : kHandleSuffixes) {
        if (name.ends_with(suffix)) {
            return true;
        }
    }
    return false;
}

class PbsdHandleVisitor : public RecursiveASTVisitor<PbsdHandleVisitor> {
public:
    explicit PbsdHandleVisitor(ASTContext& ctx) : ctx_(ctx) {
        diag_get_forbidden_ = ctx_.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "PBSD-L2: raw get() forbidden on handle types; use peek()");
        diag_copy_forbidden_ = ctx_.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "PBSD-L2: UniqueHandle copy construction is forbidden");
    }

    bool VisitCXXMemberCallExpr(CXXMemberCallExpr* call) {
        if (!call || !call->getMethodDecl()) {
            return true;
        }
        const auto* method = call->getMethodDecl();
        if (method->getName() != "get") {
            return true;
        }
        const auto* rec = method->getParent();
        if (!rec) {
            return true;
        }
        if (isPbsdHandleType(rec->getTypeForDecl()->getCanonicalTypeUnqualified())) {
            ctx_.getDiagnostics().Report(call->getBeginLoc(), diag_get_forbidden_);
        }
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr* ctor) {
        if (!ctor || !ctor->getConstructor()->isCopyConstructor()) {
            return true;
        }
        if (isPbsdHandleType(ctor->getType())) {
            ctx_.getDiagnostics().Report(ctor->getBeginLoc(), diag_copy_forbidden_);
        }
        return true;
    }

private:
    ASTContext& ctx_;
    unsigned diag_get_forbidden_;
    unsigned diag_copy_forbidden_;
};

class PbsdOwnershipConsumer : public ASTConsumer {
public:
    explicit PbsdOwnershipConsumer(ASTContext& ctx) : ctx_(ctx) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        PbsdHandleVisitor visitor(ctx);
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());
    }

private:
    ASTContext& ctx_;
};

class PbsdOwnershipAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<PbsdOwnershipConsumer>(CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance&,
                   const std::vector<std::string>&) override {
        return true;
    }
};

} // namespace

static FrontendPluginRegistry::Add<PbsdOwnershipAction>
    X("pbsd-ownership", "PBSD ownership/lifetime checker (L2)");
