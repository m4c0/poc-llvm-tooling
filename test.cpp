#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <iostream>

using namespace clang::driver;
using namespace clang;

int main() {
  llvm::StringRef clang_exe = "/usr/local/opt/llvm/bin/clang++";
  std::string triple = "wasm32-wasi";
  std::string title = "poc clang driver";

  auto diag_opts =
      IntrusiveRefCntPtr<DiagnosticOptions>{new DiagnosticOptions()};
  auto diag_ids = IntrusiveRefCntPtr<DiagnosticIDs>{new DiagnosticIDs()};
  auto diag_cli = new TextDiagnosticPrinter(llvm::errs(), &*diag_opts);

  DiagnosticsEngine diags{diag_ids, diag_opts, diag_cli};

  Driver driver{clang_exe, triple, diags, title};

  std::vector<const char *> args{};
  args.push_back("clang++");
  args.push_back("-std=c++20");
  args.push_back("-c");
  args.push_back("hello.cpp");
  args.push_back("-o");
  args.push_back("hello.o");

  auto c = driver.BuildCompilation(args);

  if (c->containsError())
    // We did a mistake in clang args. Bail out and let the diagnostics client
    // do its job informing the user
    return 1;

  llvm::SmallVector<std::pair<int, const Command *>, 4> fail_cmds;
  return driver.ExecuteCompilation(*c, fail_cmds) == 0 ? 0 : 1;
}
