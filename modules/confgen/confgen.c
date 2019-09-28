#include "confgen.h"
#include "cfg-block-generator.h"
#include "messages.h"
#include "plugin.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>


static void
confgen_set_args_as_env(gpointer k, gpointer v, gpointer user_data)
{
  gchar buf[1024];

  g_snprintf(buf, sizeof(buf), "confgen_%s", (gchar *)k);

  msg_debug("confgen: Passing argument to confgen script",
            evt_tag_str("name", (gchar *) k),
            evt_tag_str("value", (gchar *) v),
            evt_tag_str("env_name", buf));
  setenv(buf, (gchar *)v, 1);
}

static void
confgen_unset_args_from_env(gpointer k, gpointer v, gpointer user_data)
{
  gchar buf[1024];
  g_snprintf(buf, sizeof(buf), "confgen_%s", (gchar *)k);
  unsetenv(buf);
}

typedef struct _ConfgenExec
{
  CfgBlockGenerator super;
  gchar *exec;
} ConfgenExec;

static void
_read_program_output(FILE *out, GString *result)
{
  gchar buf[1024];
  gsize res;

  while ((res = fread(buf, 1, sizeof(buf), out)) > 0)
    {
      g_string_append_len(result, buf, res);
    }
}

gboolean
confgen_exec_generate(CfgBlockGenerator *s, GlobalConfig *cfg, gpointer args, GString *result, const gchar *reference)
{
  ConfgenExec *self = (ConfgenExec *) s;
  FILE *out;
  gchar buf[256];
  gint res;
  CfgArgs *cfgargs = (CfgArgs *)args;

  g_snprintf(buf, sizeof(buf), "%s confgen %s", cfg_lexer_lookup_context_name_by_type(self->super.context),
             self->super.name);

  cfg_args_foreach(cfgargs, confgen_set_args_as_env, NULL);
  out = popen(self->exec, "r");
  cfg_args_foreach(cfgargs, confgen_unset_args_from_env, NULL);

  if (!out)
    {
      msg_error("confgen: Error executing generator program",
                evt_tag_str("reference", reference),
                evt_tag_str("context", cfg_lexer_lookup_context_name_by_type(self->super.context)),
                evt_tag_str("block", self->super.name),
                evt_tag_str("exec", self->exec),
                evt_tag_error("error"));
      return FALSE;
    }

  _read_program_output(out, result);
  res = pclose(out);
  if (res != 0)
    {
      msg_error("confgen: Generator program returned with non-zero exit code",
                evt_tag_str("reference", reference),
                evt_tag_str("context", cfg_lexer_lookup_context_name_by_type(self->super.context)),
                evt_tag_str("block", self->super.name),
                evt_tag_str("exec", self->exec),
                evt_tag_int("rc", res));
      return FALSE;
    }
  msg_debug("confgen: output from the executed program to be included is",
            evt_tag_printf("block", "%.*s", (gint) result->len, result->str));
  return TRUE;
}

static void
confgen_exec_free(CfgBlockGenerator *s)
{
  ConfgenExec *self = (ConfgenExec *) s;

  g_free(self->exec);
  cfg_block_generator_free_instance(s);
}

CfgBlockGenerator *
confgen_exec_new(gint context, const gchar *name, const gchar *exec)
{
  ConfgenExec *self = g_new0(ConfgenExec, 1);

  cfg_block_generator_init_instance(&self->super, context, name);
  self->super.generate = confgen_exec_generate;
  self->super.free_fn = confgen_exec_free;
  self->exec = g_strdup(exec);
  return &self->super;
}
