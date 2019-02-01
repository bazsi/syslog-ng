/*
 * Copyright (c) 2017 Balabit
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

#include "xml.h"
#include "scratch-buffers.h"
#include "str-repr/encode.h"
#include "str-repr/decode.h"


XMLScannerOptions *
xml_parser_get_scanner_options(LogParser *p)
{
  XMLParser *self = (XMLParser *)p;
  return &self->options;
}

static void
remove_trailing_dot(gchar *str)
{
  g_assert(strlen(str));
  if (str[strlen(str)-1] == '.')
    str[strlen(str)-1] = 0;
}

static GString *
append_values(const gchar *previous_value, gssize previous_value_len,
              const gchar *current_value, gssize current_value_len, const GString *separator)
{
  GString *result = scratch_buffers_alloc();
  GString *current_encoded = scratch_buffers_alloc();
  GString *decoded = scratch_buffers_alloc();

  gchar *end, *input = (gchar *)previous_value;
  StrReprDecodeOptions options = {};
  // FIXME it seems that str-repr lib has a limit for 3 chars long separator.. have to limit text-separator as well
  memcpy(options.delimiter_chars, separator->str, MIN(3, separator->len));
  do
    {
      str_repr_decode_with_options(decoded, input, (const gchar **)&end, &options);
      if (end[0])
        input = end;
    }
  while (end[0]);
  // We have found the last element of the list
  // Let's add the other (correctly quoted) elements to the result
  g_string_append_len(result, previous_value, input - previous_value);
  // make sure that the last element is encoded
  str_repr_encode_append(result, decoded->str, decoded->len, separator->str);
  if (separator && separator->str[0])
    g_string_append_len(result, separator->str, separator->len);
  // don't forget to encode the current element (unfortunately, we cannot do that to the first element, since we only know we have a list if we found the second element)
  str_repr_encode(current_encoded, current_value, current_value_len, separator->str);
  g_string_append_len(result, current_encoded->str, current_encoded->len);
  return result;
}

typedef struct
{
  LogMessage *msg;
  GString *text_separator;
} PushParams;

static void
scanner_push_function(const gchar *name, const gchar *value, gssize value_length, gpointer user_data)
{
  PushParams *params = (PushParams *) user_data;
  LogMessage *msg = params->msg;

  gssize current_value_len = 0;
  const gchar *current_value = log_msg_get_value_by_name(msg, name, &current_value_len);
  if (current_value_len > 0)
    {
      GString *values_appended = append_values(current_value, current_value_len,
                                               value, value_length, params->text_separator);
      log_msg_set_value_by_name(msg, name, values_appended->str, values_appended->len);
      return;
    }
  log_msg_set_value_by_name(msg, name, value, value_length);
}

static gboolean
xml_parser_process(LogParser *s, LogMessage **pmsg,
                   const LogPathOptions *path_options,
                   const gchar *input, gsize input_len)
{
  XMLParser *self = (XMLParser *) s;
  LogMessage *msg = log_msg_make_writable(pmsg, path_options);
  XMLScanner xml_scanner;
  msg_trace("xml-parser message processing started",
            evt_tag_str ("input", input),
            evt_tag_str ("prefix", self->prefix),
            evt_tag_printf("msg", "%p", *pmsg));

  PushParams push_params = {.msg = msg, .text_separator = self->text_separator};
  xml_scanner_init(&xml_scanner, &self->options, &scanner_push_function, &push_params, self->prefix);

  GError *error = NULL;
  xml_scanner_parse(&xml_scanner, input, input_len, &error);
  if (error)
    goto err;

  xml_scanner_deinit(&xml_scanner);
  return TRUE;

err:
  msg_error("xml-parser failed",
            evt_tag_str("error", error->message),
            evt_tag_int("forward_invalid", self->forward_invalid));
  g_error_free(error);
  xml_scanner_deinit(&xml_scanner);
  return self->forward_invalid;
}

void
xml_parser_set_forward_invalid(LogParser *s, gboolean setting)
{
  XMLParser *self = (XMLParser *) s;
  self->forward_invalid = setting;
}

void
xml_parser_set_text_separator(LogParser *s, const gchar *separator)
{
  XMLParser *self = (XMLParser *) s;

  if (!self->text_separator)
    self->text_separator = g_string_new(separator);
  else
    {
      g_string_truncate(self->text_separator, 0);
      g_string_append(self->text_separator, separator);
    }
}

void
xml_parser_set_prefix(LogParser *s, const gchar *prefix)
{
  XMLParser *self = (XMLParser *) s;

  g_free(self->prefix);
  self->prefix = g_strdup(prefix);
}

LogPipe *
xml_parser_clone(LogPipe *s)
{
  XMLParser *self = (XMLParser *) s;
  XMLParser *cloned;

  cloned = (XMLParser *) xml_parser_new(s->cfg);

  xml_parser_set_prefix(&cloned->super, self->prefix);
  log_parser_set_template(&cloned->super, log_template_ref(self->super.template));
  xml_parser_set_forward_invalid(&cloned->super, self->forward_invalid);
  xml_parser_set_text_separator(&cloned->super, self->text_separator->str);
  xml_scanner_options_copy(&cloned->options, &self->options);

  return &cloned->super.super;
}

static void
xml_parser_free(LogPipe *s)
{
  XMLParser *self = (XMLParser *) s;
  g_free(self->prefix);
  self->prefix = NULL;
  g_string_free(self->text_separator, TRUE);
  self->text_separator = NULL;
  xml_scanner_options_destroy(&self->options);
  log_parser_free_method(s);
}


static gboolean
xml_parser_init(LogPipe *s)
{
  XMLParser *self = (XMLParser *)s;
  remove_trailing_dot(self->prefix);
  return log_parser_init_method(s);
}


LogParser *
xml_parser_new(GlobalConfig *cfg)
{
  XMLParser *self = g_new0(XMLParser, 1);

  log_parser_init_instance(&self->super, cfg);
  self->super.super.init = xml_parser_init;
  self->super.super.free_fn = xml_parser_free;
  self->super.super.clone = xml_parser_clone;
  self->super.process = xml_parser_process;
  self->forward_invalid = TRUE;

  xml_parser_set_prefix(&self->super, ".xml");
  xml_parser_set_text_separator(&self->super, ",");
  xml_scanner_options_defaults(&self->options);
  return &self->super;
}
