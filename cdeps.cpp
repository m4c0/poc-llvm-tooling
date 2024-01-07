#include <clang-c/Index.h>
#include <stdio.h>

void print(CXString str) {
  puts(clang_getCString(str));
  clang_disposeString(str);
}

auto inc_visit(CXFile included_file, CXSourceLocation *inclusion_stack,
               unsigned include_len, CXClientData client_data) {
  print(clang_getFileName(included_file));
  printf("%p\n", inclusion_stack);
  for (auto i = 0; i < include_len; i++) {
    puts("ok");
  }
}

auto visit(CXCursor cursor, CXCursor parent, CXClientData data) {
  auto mod = clang_Cursor_getModule(cursor);
  print(clang_Module_getName(mod));

  auto kind = clang_getCursorKind(cursor);

  CXString tname = clang_getCursorKindSpelling(kind);
  puts(clang_getCString(tname));
  clang_disposeString(tname);

  CXString name = clang_getCursorDisplayName(cursor);
  puts(clang_getCString(name));
  clang_disposeString(name);
  return CXChildVisit_Recurse;
}

int main() {
  constexpr const auto argc = 2;
  const char *args[argc]{"-std=c++2b", "-xc++-module"};

  auto flags =
      CXTranslationUnit_Incomplete | CXTranslationUnit_PrecompiledPreamble |
      CXTranslationUnit_ForSerialization | CXTranslationUnit_SingleFileParse;

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(index, "hello.cpp", args,
                                                      argc, nullptr, 0, flags);
  if (unit == nullptr)
    return 1;

  clang_getInclusions(unit, inc_visit, nullptr);

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  auto mod = clang_Cursor_getModule(cursor);
  print(clang_Module_getName(mod));
  clang_visitChildren(cursor, visit, nullptr);

  auto _ = CXCursor_ModuleImportDecl;
}
