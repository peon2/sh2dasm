#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>

#ifndef ARGP_H 
#define ARGP_H

#ifndef __error_t_defined
# define __error_t_defined 1
typedef int error_t;
#endif

#define ARGP_KEY_ARG		0
#define ARGP_HELP_USAGE		0x01

struct argp;
struct argp_state;	
struct argp_child;
typedef error_t (*argp_parser_t) (int __key, char *__arg, struct argp_state *__state);

extern const char *argp_program_version;

struct argp_option
{
  const char *name;
  int key;
  const char *arg;
  int flags;
  const char *doc;
  int group;
};

struct argp
{
  const struct argp_option *options;
  argp_parser_t parser;
  const char *args_doc;
  const char *doc;
};

struct argp_state
{
  const struct argp *root_argp;
  int argc; 
  char **argv;
  int next;
  void *input;
  int arg_num;
};

error_t argp_parse (const struct argp *_argp, int _argc, char **_argv, unsigned _flags, int *argv, void *_input);

void argp_usage (const struct argp_state *__restrict _state, FILE *stream);
void argp_help (const struct argp_state *__restrict __state, FILE *stream);
void argp_version ();
#endif //ARGP_H
