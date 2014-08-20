#include "template/eval.h"
#include "template/repr.h"
#include "template/macros.h"
#include "template/escaping.h"
#include "cfg.h"


void
log_template_append_format_with_context(LogTemplate *self, LogMessage **messages, gint num_messages, const LogTemplateOptions *opts, gint tz, gint32 seq_num, const gchar *context_id, GString *result)
{
  GList *p;
  LogTemplateElem *e;

  if (!opts)
    opts = &self->cfg->template_options;

  for (p = self->compiled_template; p; p = g_list_next(p))
    {
      gint msg_ndx;

      e = (LogTemplateElem *) p->data;
      if (e->text)
        {
          g_string_append_len(result, e->text, e->text_len);
        }

      /* NOTE: msg_ref is 1 larger than the index specified by the user in
       * order to make it distinguishable from the zero value.  Therefore
       * the '>' instead of '>='
       *
       * msg_ref == 0 means that the user didn't specify msg_ref
       * msg_ref >= 1 means that the user supplied the given msg_ref, 1 is equal to @0 */
      if (e->msg_ref > num_messages)
        continue;
      msg_ndx = num_messages - e->msg_ref;

      /* value and macro can't understand a context, assume that no msg_ref means @0 */
      if (e->msg_ref == 0)
        msg_ndx--;

      switch (e->type)
        {
        case LTE_VALUE:
          {
            const gchar *value = NULL;
            gssize value_len = -1;

            value = log_msg_get_value(messages[msg_ndx], e->value_handle, &value_len);
            if (value && value[0])
              result_append(result, value, value_len, self->escape);
            else if (e->default_value)
              result_append(result, e->default_value, -1, self->escape);
            break;
          }
        case LTE_MACRO:
          {
            gint len = result->len;

            if (e->macro)
              {
                log_macro_expand(result, e->macro, self->escape, opts ? opts : &self->cfg->template_options, tz, seq_num, context_id, messages[msg_ndx]);
                if (len == result->len && e->default_value)
                  g_string_append(result, e->default_value);
              }
            break;
          }
        case LTE_FUNC:
          {
            g_static_mutex_lock(&self->arg_lock);
            if (!self->arg_bufs)
              self->arg_bufs = g_ptr_array_sized_new(0);

            if (1)
              {
                LogTemplateInvokeArgs args =
                  {
                    .super = {
                      .messages = e->msg_ref ? &messages[msg_ndx] : messages,
                      .num_messages = e->msg_ref ? 1 : num_messages,
                      .opts = opts,
                      .tz = tz,
                      .seq_num = seq_num,
                      .context_id = context_id
                    },
                    .bufs = self->arg_bufs,
                  };


                /* if a function call is called with an msg_ref, we only
                 * pass that given logmsg to argument resolution, otherwise
                 * we pass the whole set so the arguments can individually
                 * specify which message they want to resolve from
                 */
                if (e->func.ops->eval)
                  e->func.ops->eval(e->func.ops, e->func.state, &args);
                e->func.ops->call(e->func.ops, e->func.state, &args, result);
              }
            g_static_mutex_unlock(&self->arg_lock);
            break;
          }
        }
    }
}

void
log_template_format_with_context(LogTemplate *self, LogMessage **messages, gint num_messages, const LogTemplateOptions *opts, gint tz, gint32 seq_num, const gchar *context_id, GString *result)
{
  g_string_truncate(result, 0);
  log_template_append_format_with_context(self, messages, num_messages, opts, tz, seq_num, context_id, result);
}

void
log_template_append_format(LogTemplate *self, LogMessage *lm, const LogTemplateOptions *opts, gint tz, gint32 seq_num, const gchar *context_id, GString *result)
{
  log_template_append_format_with_context(self, &lm, 1, opts, tz, seq_num, context_id, result);
}

void
log_template_format(LogTemplate *self, LogMessage *lm, const LogTemplateOptions *opts, gint tz, gint32 seq_num, const gchar *context_id, GString *result)
{
  g_string_truncate(result, 0);
  log_template_append_format(self, lm, opts, tz, seq_num, context_id, result);
}


void
log_template_append_format_recursive(LogTemplate *self, const LogTemplateEvalArgs *args, GString *result)
{
  log_template_append_format_with_context(self,
                                          args->messages, args->num_messages,
                                          args->opts, args->tz, args->seq_num, args->context_id, result);
}
