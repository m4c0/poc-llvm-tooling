#include "clang/Tooling/DependencyScanning/DependencyScanningService.h"
#include "clang/Tooling/DependencyScanning/DependencyScanningTool.h"
#include <iostream>

using namespace clang::tooling::dependencies;

int main() {
  auto scan_mode = ScanningMode::DependencyDirectivesScan;
  auto format = ScanningOutputFormat::P1689;
  auto opt_args = false;
  auto eager_load_mod = false;

  auto service =
      DependencyScanningService{scan_mode, format, opt_args, eager_load_mod};
  auto tool = DependencyScanningTool{service};

  /////////////////////////////////////////////////////////////////////////////
  // This block comes from the DB in clang-scan-deps
  /////////////////////////////////////////////////////////////////////////////
  clang::Twine dir{"."};
  clang::Twine file{"hello.cpp"};
  clang::Twine output{"hello.o"};

  std::vector<std::string> cmd_line{};
  cmd_line.push_back("clang++");
  cmd_line.push_back("-std=c++20");
  cmd_line.push_back("hello.cpp");
  cmd_line.push_back("-o");
  cmd_line.push_back("hello.o");

  clang::tooling::CompileCommand input{".", "hello.cpp", cmd_line, output};
  std::string cwd{"."};
  /////////////////////////////////////////////////////////////////////////////

  std::string mf_out{};
  std::string mf_out_path{};
  auto rule =
      tool.getP1689ModuleDependencyFile(input, cwd, mf_out, mf_out_path);
  if (!rule) {
    llvm::handleAllErrors(rule.takeError(), [&](llvm::StringError &err) {
      std::cerr << err.getMessage();
    });
    return 1;
  }

  std::cout << "primary-output: " << rule->PrimaryOutput << "\n";
  if (rule->Provides) {
    std::cout << "provides: " << rule->Provides->ModuleName << "\n";
  }
  for (auto &req : rule->Requires) {
    std::cout << "requires: " << req.ModuleName << "\n";
    std::cout << "     src: " << req.SourcePath << "\n";
  }
}
