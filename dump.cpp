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
  ExamplePragmaHandler() : PragmaHandler("uga") {}
  void HandlePragma(Preprocessor &PP, PragmaIntroducer Introducer,
                    Token &PragmaTok) {
    Token Tok;
    PP.LexUnexpandedToken(Tok);
    if (!Tok.isAnyIdentifier())
      PP.Diag(Tok, diag::err_pp_identifier_arg_not_identifier) << Tok.getKind();
  }
};

class MyAction : public PreprocessorFrontendAction {
public:
  bool BeginSourceFileAction(CompilerInstance &CI) override {
    Preprocessor &PP = getCompilerInstance().getPreprocessor();

    PP.EnterMainSourceFile();
    PP.AddPragmaHandler("example", new ExamplePragmaHandler());

    Token Tok;
    do {
      PP.Lex(Tok);
    } while (Tok.isNot(tok::eof));

    return true;
  }
  void ExecuteAction() override {}
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

  clang::tooling::ToolInvocation tool{args, std::make_unique<MyAction>(),
                                      &*files, pch_opts};
  return tool.run() ? 0 : 1;
}
