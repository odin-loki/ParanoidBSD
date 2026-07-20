// PBSD Static Analyser — Layer 4 ISR Context (Wave 1 stub)
// PROVENANCE: Clang LibTooling public API.
// SI-5: functions annotated [[clang::annotate("pbsd::isr")]] must not call
//       functions annotated [[clang::annotate("pbsd::blocking")]].

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

using namespace clang;

namespace {

bool hasAnnotate(const Decl* decl, llvm::StringRef tag) {
    if (!decl) {
        return false;
    }
    for (const auto* attr : decl->attrs()) {
        if (const auto* ann = dyn_cast<AnnotateAttr>(attr)) {
            if (ann->getAnnotation() == tag) {
                return true;
            }
        }
    }
    return false;
}

bool isIsrContext(const FunctionDecl* fn) {
    return fn && hasAnnotate(fn, "pbsd::isr");
}

bool isBlockingCallee(const FunctionDecl* fn) {
    return fn && hasAnnotate(fn, "pbsd::blocking");
}

class PbsdIsrVisitor : public RecursiveASTVisitor<PbsdIsrVisitor> {
public:
    explicit PbsdIsrVisitor(ASTContext& ctx, FunctionDecl* isr_fn)
        : ctx_(ctx), isr_fn_(isr_fn) {
        diag_blocking_in_isr_ = ctx_.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error,
            "PBSD-L4: blocking call in ISR '%0' violates SI-5");
    }

    bool VisitCallExpr(CallExpr* call) {
        if (!call) {
            return true;
        }
        const auto* callee = call->getDirectCallee();
        if (!isBlockingCallee(callee)) {
            return true;
        }
        ctx_.getDiagnostics().Report(call->getBeginLoc(), diag_blocking_in_isr_)
            << isr_fn_->getName();
        return true;
    }

private:
    ASTContext& ctx_;
    FunctionDecl* isr_fn_;
    unsigned diag_blocking_in_isr_;
};

class PbsdIsrConsumer : public ASTConsumer {
public:
    explicit PbsdIsrConsumer(ASTContext& ctx) : ctx_(ctx) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        for (Decl* decl : ctx.getTranslationUnitDecl()->decls()) {
            auto* fn = dyn_cast<FunctionDecl>(decl);
            if (!isIsrContext(fn)) {
                continue;
            }
            PbsdIsrVisitor visitor(ctx, fn);
            visitor.TraverseDecl(fn);
        }
    }

private:
    ASTContext& ctx_;
};

class PbsdIsrAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<PbsdIsrConsumer>(CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance&,
                   const std::vector<std::string>&) override {
        return true;
    }
};

} // namespace

static FrontendPluginRegistry::Add<PbsdIsrAction>
    Y("pbsd-isr", "PBSD ISR-context checker (L4)");
