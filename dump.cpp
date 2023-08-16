#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

// Define a pragma handler for #pragma example_pragma
class ExamplePragmaHandler : public PragmaHandler {
public:
  ExamplePragmaHandler() : PragmaHandler("example_pragma") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducer Introducer,
                    Token &PragmaTok) {
    Token Tok;
    PP.LexUnexpandedToken(Tok);
    if (Tok.isNot(tok::eod))
      PP.Diag(Tok, diag::ext_pp_extra_tokens_at_eol) << "pragma";
  }
};

static PragmaHandlerRegistry::Add<ExamplePragmaHandler>
    Y("example_pragma", "example pragma description");

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context) : Context(Context) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString() == "n::m::C") {
      FullSourceLoc FullLocation =
          Context->getFullLoc(Declaration->getBeginLoc());
      if (FullLocation.isValid())
        llvm::outs() << "Found declaration at "
                     << FullLocation.getSpellingLineNumber() << ":"
                     << FullLocation.getSpellingColumnNumber() << "\n";
    }
    return true;
  }

private:
  ASTContext *Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *Context) : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext());
  }
};

int main() {
  clang::FileSystemOptions opts{
      .WorkingDir = {},
  };
  auto files = IntrusiveRefCntPtr<FileManager>(new FileManager(opts));

  auto pch_opts = std::make_shared<clang::PCHContainerOperations>();

  std::vector<std::string> args{};
  args.push_back("clang++");
  args.push_back("-std=c++20");
  args.push_back("-c");
  args.push_back("hello.cpp");
  args.push_back("-o");
  args.push_back("hello.o");

  clang::tooling::ToolInvocation tool{
      args, std::make_unique<FindNamedClassAction>(), &*files, pch_opts};
  return tool.run() ? 0 : 1;
}
