#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/Preprocessor.h"

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
