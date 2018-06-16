/*
 * Copyright (c) 2018 Balabit
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "cfg-lexer.h"
#include "cfg-grammar.h"
#include "apphook.h"
#include <criterion/criterion.h>

#define TESTDATA_DIR TOP_SRCDIR "/modules/confgen/tests"

typedef struct
{
  YYSTYPE *yylval;
  YYLTYPE *yylloc;
  CfgLexer *lexer;
} TestParser;

TestParser *parser = NULL;

static void
test_parser_clear_token(TestParser *self)
{
  if (self->yylval->type)
    cfg_lexer_free_token(self->yylval);
  self->yylval->type = 0;
}

static void
test_parser_next_token(TestParser *self)
{
  test_parser_clear_token(self);
  cfg_lexer_lex(self->lexer, self->yylval, self->yylloc);
}

static void
test_parser_input(TestParser *self, const gchar *buffer)
{
  cfg_lexer_include_buffer(self->lexer, "#test-buffer", buffer, strlen(buffer));
}

TestParser *
test_parser_new(void)
{
  TestParser *self = g_new0(TestParser, 1);

  self->yylval = g_new0(YYSTYPE, 1);
  self->yylloc = g_new0(YYLTYPE, 1);
  self->yylval->type = LL_CONTEXT_ROOT;
  self->yylloc->first_column = 1;
  self->yylloc->first_line = 1;
  self->yylloc->last_column = 1;
  self->yylloc->last_line = 1;
  self->yylloc->level = &self->lexer->include_stack[0];

  self->lexer = cfg_lexer_new_buffer(configuration, "", 0);
  return self;
}

void
test_parser_free(TestParser *self)
{
  test_parser_clear_token(self);
  if (self->lexer)
    cfg_lexer_free(self->lexer);
  g_free(self->yylval);
  g_free(self->yylloc);
  g_free(self);
}

static void
_input(const gchar *input)
{
  test_parser_input(parser, input);
}

static void
_next_token(void)
{
  test_parser_next_token(parser);
}

static YYSTYPE *
_current_token(void)
{
  return parser->yylval;
}

#define assert_token_type(expected)                                     \
  cr_assert_eq(_current_token()->type, expected, "Unexpected token type %d != %d", _current_token()->type, expected);

#define assert_parser_string(expected)                          \
  _next_token();                                                        \
  assert_token_type(LL_STRING);                                        \
  cr_assert_str_eq(_current_token()->cptr, expected, "Unexpected string value parsed >>>%s<<< != >>>%s<<<", _current_token()->cptr, expected);

#define assert_parser_identifier(expected) \
  _next_token();                                                        \
  assert_token_type(LL_IDENTIFIER);                                         \
  cr_assert_str_eq(_current_token()->cptr, expected, "Unexpected identifier parsed >>>%s<<< != >>>%s<<<", _current_token()->cptr, expected);


Test(confgen, confgen_script_output_is_included_into_the_config)
{
  parser->lexer->ignore_pragma = FALSE;

  cfg_lexer_push_context(parser->lexer, main_parser.context, main_parser.keywords, main_parser.name);
  _input("@version: 3.16\n"
"@module confgen context(root) name(confgentest) exec("TESTDATA_DIR "/confgentest.sh)\n"
"confgentest()");

  assert_parser_identifier("from-confgen1");
  assert_parser_identifier("from-confgen2");
  cfg_lexer_pop_context(parser->lexer);
}

static void
setup(void)
{
  app_startup();
  configuration = cfg_new_snippet();
  parser = test_parser_new();
}

static void
teardown(void)
{
  test_parser_free(parser);
  cfg_free(configuration);
  configuration = NULL;
  app_shutdown();
}

TestSuite(confgen, .init = setup, .fini = teardown);
