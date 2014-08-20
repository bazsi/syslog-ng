#ifndef TEMPLATE_EVAL_H_INCLUDED
#define TEMPLATE_EVAL_H_INCLUDED 1

#include "syslog-ng.h"
#include "timeutils.h"

#define LTZ_LOCAL 0
#define LTZ_SEND  1
#define LTZ_MAX   2

typedef struct _LogTemplate LogTemplate;

/* template expansion options that can be influenced by the user and
 * is static throughout the runtime for a given configuration. There
 * are call-site specific options too, those are specified as
 * arguments to log_template_format() */
typedef struct _LogTemplateOptions
{
  gboolean initialized;
  /* timestamp format as specified by ts_format() */
  gint ts_format;
  /* number of digits in the fraction of a second part, specified using frac_digits() */
  gint frac_digits;

  /* timezone for LTZ_LOCAL/LTZ_SEND settings */
  gchar *time_zone[LTZ_MAX];
  TimeZoneInfo *time_zone_info[LTZ_MAX];

  /* Template error handling settings */
  gint on_error;
} LogTemplateOptions;

typedef struct _LogTemplateEvalArgs
{
  /* context in case of correllation */
  LogMessage **messages;
  gint num_messages;

  /* options for recursive template evaluation, inherited from the parent */
  const LogTemplateOptions *opts;
  gint tz;
  gint seq_num;
  const gchar *context_id;
} LogTemplateEvalArgs;

#endif
