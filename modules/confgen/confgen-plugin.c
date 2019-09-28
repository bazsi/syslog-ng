/*
 * Copyright (c) 2002-2011 Balabit
 * Copyright (c) 1998-2011 Balázs Scheidler
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

#include "confgen.h"
#include "plugin-types.h"
#include "cfg-parser.h"

extern CfgParser confgen_parser;

static Plugin confgen_plugins[] =
{
  {
    .type = LL_CONTEXT_SOURCE | LL_CONTEXT_FLAG_GENERATOR,
    .name = "confgen",
    .parser = &confgen_parser
  },
};

gboolean
confgen_module_init(PluginContext *plugin_context, CfgArgs *args)
{
  const gchar *name, *context, *exec;
  gint context_value;

  if (!args)
    {
      msg_trace("confgen: no arguments, registering non-instantiated plugins");
      plugin_register(plugin_context, confgen_plugins, G_N_ELEMENTS(confgen_plugins));
      return TRUE;
    }

  name = cfg_args_get(args, "name");
  if (!name)
    {
      msg_error("confgen: name argument expected");
      return FALSE;
    }
  context = cfg_args_get(args, "context");
  if (!context)
    {
      msg_error("confgen: context argument expected");
      return FALSE;
    }
  context_value = cfg_lexer_lookup_context_type_by_name(context);
  if (context_value == 0)
    {
      msg_error("confgen: context value is unknown",
                evt_tag_str("context", context));
      return FALSE;
    }
  exec = cfg_args_get(args, "exec");
  if (!exec)
    {
      msg_error("confgen: exec argument expected");
      return FALSE;
    }
  cfg_lexer_register_generator_plugin(plugin_context,
                                      confgen_exec_new(context_value, name, exec));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "confgen",
  .version = SYSLOG_NG_VERSION,
  .description = "The confgen module provides support for dynamically generated configuration file snippets for syslog-ng, used for the SCL system() driver for example",
  .core_revision = SYSLOG_NG_SOURCE_REVISION,
  .plugins = confgen_plugins,
  .plugins_len = G_N_ELEMENTS(confgen_plugins),
};
