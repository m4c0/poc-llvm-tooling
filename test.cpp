#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <iostream>

using namespace clang::driver;
using namespace clang;

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

class MyAction : public EmitObjAction {
public:
  bool BeginSourceFileAction(CompilerInstance &CI) override {
    CI.getPreprocessor().AddPragmaHandler("example",
                                          new ExamplePragmaHandler());

    return EmitObjAction::BeginSourceFileAction(CI);
  }
};

int main() {
  constexpr const auto clang_exe = "/usr/local/opt/llvm/bin/clang++";
  std::string triple = "wasm32-wasi";
  std::string title = "poc clang driver";

  auto diag_opts =
      IntrusiveRefCntPtr<DiagnosticOptions>{new DiagnosticOptions()};
  auto diag_ids = IntrusiveRefCntPtr<DiagnosticIDs>{new DiagnosticIDs()};
  auto diag_cli = new TextDiagnosticPrinter(llvm::errs(), &*diag_opts);

  DiagnosticsEngine diags{diag_ids, diag_opts, diag_cli};
  Driver driver{clang_exe, triple, diags, title};

  std::vector<const char *> args{};
  args.push_back(clang_exe);
  args.push_back("-std=c++20");
  args.push_back("-c");
  args.push_back("hello.cpp");
  args.push_back("-o");
  args.push_back("hello.o");

  auto c = std::unique_ptr<Compilation>{driver.BuildCompilation(args)};
  if (!c || c->containsError())
    // We did a mistake in clang args. Bail out and let the diagnostics client
    // do its job informing the user
    return 1;

  auto cc1_args = c->getJobs().getJobs()[0]->getArguments();

  auto files = IntrusiveRefCntPtr<FileManager>(new FileManager({}));
  auto cdiag =
      CompilerInstance::createDiagnostics(&*diag_opts, diag_cli, false);

  SourceManager src_mgr(*cdiag, *files);
  cdiag->setSourceManager(&src_mgr);

  auto cinv = std::make_shared<CompilerInvocation>();
  CompilerInvocation::CreateFromArgs(*cinv, cc1_args, *cdiag);

  auto pch_opts = std::make_shared<clang::PCHContainerOperations>();
  CompilerInstance cinst{pch_opts};
  cinst.setInvocation(cinv);
  cinst.setFileManager(&*files);
  cinst.createDiagnostics(diag_cli, false);
  cinst.createSourceManager(*files);

  MyAction a{};
  cinst.ExecuteAction(a);

  files->clearStatCache();

  // llvm::SmallVector<std::pair<int, const Command *>, 4> fail_cmds;
  // auto result = driver.ExecuteCompilation(*c, fail_cmds);

  llvm::llvm_shutdown();
  // return result == 0 ? 0 : 1;
}
