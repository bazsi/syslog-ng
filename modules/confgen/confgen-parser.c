/*
 * Copyright (c) 2014-2015 Balabit
 * Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>
 * Copyright (c) 2015 Balazs Scheidler <balazs.scheidler@balabit.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "confgen-parser.h"
#include "confgen-grammar.h"

extern int confgen_debug;
int confgen_parse(CfgLexer *lexer, void **instance, gpointer arg);

static CfgLexerKeyword confgen_keywords[] =
{
  { "confgen",                   KW_CONFGEN  },
  { NULL }
};

CfgParser confgen_parser =
{
#if SYSLOG_NG_ENABLE_DEBUG
  .debug_flag = &confgen_debug,
#endif
  .name = "confgen",
  .keywords = confgen_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) confgen_parse,
  .cleanup = (void (*)(gpointer)) cfg_block_generator_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(confgen_, void **)
