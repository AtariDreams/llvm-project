from __future__ import print_function

import copy
import glob
import re
import subprocess
import sys

if sys.version_info[0] > 2:
  class string:
    expandtabs = str.expandtabs
else:
  import string

##### Common utilities for update_*test_checks.py


_verbose = False

def parse_commandline_args(parser):
  parser.add_argument('-v', '--verbose', action='store_true',
                      help='Show verbose output')
  parser.add_argument('-u', '--update-only', action='store_true',
                      help='Only update test if it was already autogened')
  parser.add_argument('--force-update', action='store_true',
                      help='Update test even if it was autogened by a different script')
  parser.add_argument('--enable', action='store_true', dest='enabled', default=True,
                       help='Activate CHECK line generation from this point forward')
  parser.add_argument('--disable', action='store_false', dest='enabled',
                      help='Deactivate CHECK line generation from this point forward')
  args = parser.parse_args()
  global _verbose
  _verbose = args.verbose
  return args


class InputLineInfo(object):
  def __init__(self, line, line_number, args, argv):
    self.line = line
    self.line_number = line_number
    self.args = args
    self.argv = argv


class TestInfo(object):
  def __init__(self, test, parser, script_name, input_lines, args, argv,
               comment_prefix, argparse_callback):
    self.parser = parser
    self.argparse_callback = argparse_callback
    self.path = test
    self.args = args
    self.argv = argv
    self.input_lines = input_lines
    self.run_lines = find_run_lines(test, self.input_lines)
    self.comment_prefix = comment_prefix
    if self.comment_prefix is None:
      if self.path.endswith('.mir'):
        self.comment_prefix = '#'
      else:
        self.comment_prefix = ';'
    self.autogenerated_note_prefix = self.comment_prefix + ' ' + UTC_ADVERT
    self.test_autogenerated_note = self.autogenerated_note_prefix + script_name
    self.test_autogenerated_note += get_autogennote_suffix(parser, self.args)

  def iterlines(self, output_lines):
    output_lines.append(self.test_autogenerated_note)
    for line_num, input_line in enumerate(self.input_lines):
      # Discard any previous script advertising.
      if input_line.startswith(self.autogenerated_note_prefix):
        continue
      self.args, self.argv = check_for_command(input_line, self.parser,
                                               self.args, self.argv, self.argparse_callback)
      if not self.args.enabled:
        output_lines.append(input_line)
        continue
      yield InputLineInfo(input_line, line_num, self.args, self.argv)


def itertests(test_patterns, parser, script_name, comment_prefix=None, argparse_callback=None):
  for pattern in test_patterns:
    # On Windows we must expand the patterns ourselves.
    tests_list = glob.glob(pattern)
    if not tests_list:
      warn("Test file pattern '%s' was not found. Ignoring it." % (pattern,))
      continue
    for test in tests_list:
      with open(test) as f:
        input_lines = [l.rstrip() for l in f]
      args = parser.parse_args()
      if argparse_callback is not None:
        argparse_callback(args)
      argv = sys.argv[:]
      first_line = input_lines[0] if input_lines else ""
      if UTC_ADVERT in first_line:
        if script_name not in first_line and not args.force_update:
          warn("Skipping test which wasn't autogenerated by " + script_name, test)
          continue
        args, argv = check_for_command(first_line, parser, args, argv, argparse_callback)
      elif args.update_only:
        assert UTC_ADVERT not in first_line
        warn("Skipping test which isn't autogenerated: " + test)
        continue
      yield TestInfo(test, parser, script_name, input_lines, args, argv,
                     comment_prefix, argparse_callback)


def should_add_line_to_output(input_line, prefix_set):
  # Skip any blank comment lines in the IR.
  if input_line.strip() == ';':
    return False
  # Skip any blank lines in the IR.
  #if input_line.strip() == '':
  #  return False
  # And skip any CHECK lines. We're building our own.
  m = CHECK_RE.match(input_line)
  if m and m.group(1) in prefix_set:
    return False

  return True

# Invoke the tool that is being tested.
def invoke_tool(exe, cmd_args, ir):
  with open(ir) as ir_file:
    # TODO Remove the str form which is used by update_test_checks.py and
    # update_llc_test_checks.py
    # The safer list form is used by update_cc_test_checks.py
    if isinstance(cmd_args, list):
      stdout = subprocess.check_output([exe] + cmd_args, stdin=ir_file)
    else:
      stdout = subprocess.check_output(exe + ' ' + cmd_args,
                                       shell=True, stdin=ir_file)
    if sys.version_info[0] > 2:
      stdout = stdout.decode()
  # Fix line endings to unix CR style.
  return stdout.replace('\r\n', '\n')

##### LLVM IR parser
RUN_LINE_RE = re.compile(r'^\s*(?://|[;#])\s*RUN:\s*(.*)$')
CHECK_PREFIX_RE = re.compile(r'--?check-prefix(?:es)?[= ](\S+)')
PREFIX_RE = re.compile('^[a-zA-Z0-9_-]+$')
CHECK_RE = re.compile(r'^\s*(?://|[;#])\s*([^:]+?)(?:-NEXT|-NOT|-DAG|-LABEL|-SAME|-EMPTY)?:')

UTC_ARGS_KEY = 'UTC_ARGS:'
UTC_ARGS_CMD = re.compile(r'.*' + UTC_ARGS_KEY + '\s*(?P<cmd>.*)\s*$')
UTC_ADVERT = 'NOTE: Assertions have been autogenerated by '

OPT_FUNCTION_RE = re.compile(
    r'^(\s*;\s*Function\sAttrs:\s(?P<attrs>[\w\s]+?))?\s*define\s+(?:internal\s+)?[^@]*@(?P<func>[\w.-]+?)\s*'
    r'(?P<args_and_sig>\((\)|(.*?[\w.-]+?)\))[^{]*\{)\n(?P<body>.*?)^\}$',
    flags=(re.M | re.S))

ANALYZE_FUNCTION_RE = re.compile(
    r'^\s*\'(?P<analysis>[\w\s-]+?)\'\s+for\s+function\s+\'(?P<func>[\w.-]+?)\':'
    r'\s*\n(?P<body>.*)$',
    flags=(re.X | re.S))

IR_FUNCTION_RE = re.compile(r'^\s*define\s+(?:internal\s+)?[^@]*@([\w.-]+)\s*\(')
TRIPLE_IR_RE = re.compile(r'^\s*target\s+triple\s*=\s*"([^"]+)"$')
TRIPLE_ARG_RE = re.compile(r'-mtriple[= ]([^ ]+)')
MARCH_ARG_RE = re.compile(r'-march[= ]([^ ]+)')

SCRUB_LEADING_WHITESPACE_RE = re.compile(r'^(\s+)')
SCRUB_WHITESPACE_RE = re.compile(r'(?!^(|  \w))[ \t]+', flags=re.M)
SCRUB_TRAILING_WHITESPACE_RE = re.compile(r'[ \t]+$', flags=re.M)
SCRUB_TRAILING_WHITESPACE_TEST_RE = SCRUB_TRAILING_WHITESPACE_RE
SCRUB_TRAILING_WHITESPACE_AND_ATTRIBUTES_RE = re.compile(r'([ \t]|(#[0-9]+))+$', flags=re.M)
SCRUB_KILL_COMMENT_RE = re.compile(r'^ *#+ +kill:.*\n')
SCRUB_LOOP_COMMENT_RE = re.compile(
    r'# =>This Inner Loop Header:.*|# in Loop:.*', flags=re.M)
SCRUB_TAILING_COMMENT_TOKEN_RE = re.compile(r'(?<=\S)+[ \t]*#$', flags=re.M)


def error(msg, test_file=None):
  if test_file:
    msg = '{}: {}'.format(msg, test_file)
  print('ERROR: {}'.format(msg), file=sys.stderr)

def warn(msg, test_file=None):
  if test_file:
    msg = '{}: {}'.format(msg, test_file)
  print('WARNING: {}'.format(msg), file=sys.stderr)

def debug(*args, **kwargs):
  # Python2 does not allow def debug(*args, file=sys.stderr, **kwargs):
  if 'file' not in kwargs:
    kwargs['file'] = sys.stderr
  if _verbose:
    print(*args, **kwargs)

def find_run_lines(test, lines):
  debug('Scanning for RUN lines in test file:', test)
  raw_lines = [m.group(1)
               for m in [RUN_LINE_RE.match(l) for l in lines] if m]
  run_lines = [raw_lines[0]] if len(raw_lines) > 0 else []
  for l in raw_lines[1:]:
    if run_lines[-1].endswith('\\'):
      run_lines[-1] = run_lines[-1].rstrip('\\') + ' ' + l
    else:
      run_lines.append(l)
  debug('Found {} RUN lines in {}:'.format(len(run_lines), test))
  for l in run_lines:
    debug('  RUN: {}'.format(l))
  return run_lines

def scrub_body(body):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  body = SCRUB_WHITESPACE_RE.sub(r' ', body)
  # Expand the tabs used for indentation.
  body = string.expandtabs(body, 2)
  # Strip trailing whitespace.
  body = SCRUB_TRAILING_WHITESPACE_TEST_RE.sub(r'', body)
  return body

def do_scrub(body, scrubber, scrubber_args, extra):
  if scrubber_args:
    local_args = copy.deepcopy(scrubber_args)
    local_args[0].extra_scrub = extra
    return scrubber(body, *local_args)
  return scrubber(body, *scrubber_args)

# Build up a dictionary of all the function bodies.
class function_body(object):
  def __init__(self, string, extra, args_and_sig, attrs):
    self.scrub = string
    self.extrascrub = extra
    self.args_and_sig = args_and_sig
    self.attrs = attrs
  def is_same_except_arg_names(self, extrascrub, args_and_sig, attrs):
    arg_names = set()
    def drop_arg_names(match):
        arg_names.add(match.group(3))
        return match.group(1) + match.group(match.lastindex)
    def repl_arg_names(match):
        if match.group(3) in arg_names:
            return match.group(1) + match.group(match.lastindex)
        return match.group(1) + match.group(2) + match.group(match.lastindex)
    if self.attrs != attrs:
      return False
    ans0 = IR_VALUE_RE.sub(drop_arg_names, self.args_and_sig)
    ans1 = IR_VALUE_RE.sub(drop_arg_names, args_and_sig)
    if ans0 != ans1:
        return False
    es0 = IR_VALUE_RE.sub(repl_arg_names, self.extrascrub)
    es1 = IR_VALUE_RE.sub(repl_arg_names, extrascrub)
    es0 = SCRUB_IR_COMMENT_RE.sub(r'', es0)
    es1 = SCRUB_IR_COMMENT_RE.sub(r'', es1)
    return es0 == es1

  def __str__(self):
    return self.scrub

def build_function_body_dictionary(function_re, scrubber, scrubber_args, raw_tool_output, prefixes, func_dict, verbose, record_args, check_attributes):
  for m in function_re.finditer(raw_tool_output):
    if not m:
      continue
    func = m.group('func')
    body = m.group('body')
    attrs = m.group('attrs') if check_attributes else ''
    # Determine if we print arguments, the opening brace, or nothing after the function name
    if record_args and 'args_and_sig' in m.groupdict():
        args_and_sig = scrub_body(m.group('args_and_sig').strip())
    elif 'args_and_sig' in m.groupdict():
        args_and_sig = '('
    else:
        args_and_sig = ''
    scrubbed_body = do_scrub(body, scrubber, scrubber_args, extra = False)
    scrubbed_extra = do_scrub(body, scrubber, scrubber_args, extra = True)
    if 'analysis' in m.groupdict():
      analysis = m.group('analysis')
      if analysis.lower() != 'cost model analysis':
        warn('Unsupported analysis mode: %r!' % (analysis,))
    if func.startswith('stress'):
      # We only use the last line of the function body for stress tests.
      scrubbed_body = '\n'.join(scrubbed_body.splitlines()[-1:])
    if verbose:
      print('Processing function: ' + func, file=sys.stderr)
      for l in scrubbed_body.splitlines():
        print('  ' + l, file=sys.stderr)
    for prefix in prefixes:
      if func in func_dict[prefix]:
        if str(func_dict[prefix][func]) != scrubbed_body or (func_dict[prefix][func] and (func_dict[prefix][func].args_and_sig != args_and_sig or func_dict[prefix][func].attrs != attrs)):
          if func_dict[prefix][func] and func_dict[prefix][func].is_same_except_arg_names(scrubbed_extra, args_and_sig, attrs):
            func_dict[prefix][func].scrub = scrubbed_extra
            func_dict[prefix][func].args_and_sig = args_and_sig
            continue
          else:
            if prefix == prefixes[-1]:
              warn('Found conflicting asm under the same prefix: %r!' % (prefix,))
            else:
              func_dict[prefix][func] = None
              continue

      func_dict[prefix][func] = function_body(scrubbed_body, scrubbed_extra, args_and_sig, attrs)

##### Generator of LLVM IR CHECK lines

SCRUB_IR_COMMENT_RE = re.compile(r'\s*;.*')

# TODO: We should also derive check lines for global, debug, loop declarations, etc..

class NamelessValue:
    def __init__(self, check_prefix, ir_prefix, ir_regexp):
        self.check_prefix = check_prefix
        self.ir_prefix = ir_prefix
        self.ir_regexp = ir_regexp

# Description of the different "unnamed" values we match in the IR, e.g.,
# (local) ssa values, (debug) metadata, etc.
nameless_values = [
    NamelessValue(r'TMP',   r'%',            r'[\w.-]+?'),
    NamelessValue(r'GLOB',  r'@',            r'[0-9]+?'),
    NamelessValue(r'ATTR',  r'#',            r'[0-9]+?'),
    NamelessValue(r'DBG',   r'!dbg !',       r'[0-9]+?'),
    NamelessValue(r'TBAA',  r'!tbaa !',      r'[0-9]+?'),
    NamelessValue(r'RNG',   r'!range !',     r'[0-9]+?'),
    NamelessValue(r'LOOP',  r'!llvm.loop !', r'[0-9]+?'),
    NamelessValue(r'META',  r'metadata !',   r'[0-9]+?'),
]

# Build the regexp that matches an "IR value". This can be a local variable,
# argument, global, or metadata, anything that is "named". It is important that
# the PREFIX and SUFFIX below only contain a single group, if that changes
# other locations will need adjustment as well.
IR_VALUE_REGEXP_PREFIX = r'(\s+)'
IR_VALUE_REGEXP_STRING = r''
for nameless_value in nameless_values:
    if IR_VALUE_REGEXP_STRING:
        IR_VALUE_REGEXP_STRING += '|'
    IR_VALUE_REGEXP_STRING += nameless_value.ir_prefix + r'(' + nameless_value.ir_regexp + r')'
IR_VALUE_REGEXP_SUFFIX = r'([,\s\(\)]|\Z)'
IR_VALUE_RE = re.compile(IR_VALUE_REGEXP_PREFIX + r'(' + IR_VALUE_REGEXP_STRING + r')' + IR_VALUE_REGEXP_SUFFIX)

# The entire match is group 0, the prefix has one group (=1), the entire
# IR_VALUE_REGEXP_STRING is one group (=2), and then the nameless values start.
first_nameless_group_in_ir_value_match = 3

# Check a match for IR_VALUE_RE and inspect it to determine if it was a local
# value, %..., global @..., debug number !dbg !..., etc. See the PREFIXES above.
def get_idx_from_ir_value_match(match):
    for i in range(first_nameless_group_in_ir_value_match, match.lastindex):
        if match.group(i) is not None:
            return i - first_nameless_group_in_ir_value_match
    error("Unable to identify the kind of IR value from the match!")
    return 0;

# See get_idx_from_ir_value_match
def get_name_from_ir_value_match(match):
    return match.group(get_idx_from_ir_value_match(match) + first_nameless_group_in_ir_value_match)

# Return the nameless prefix we use for this kind or IR value, see also
# get_idx_from_ir_value_match
def get_nameless_check_prefix_from_ir_value_match(match):
    return nameless_values[get_idx_from_ir_value_match(match)].check_prefix

# Return the IR prefix we use for this kind or IR value, e.g., % for locals,
# see also get_idx_from_ir_value_match
def get_ir_prefix_from_ir_value_match(match):
    return nameless_values[get_idx_from_ir_value_match(match)].ir_prefix

# Return true if this kind or IR value is "local", basically if it matches '%{{.*}}'.
def is_local_ir_value_match(match):
    return nameless_values[get_idx_from_ir_value_match(match)].ir_prefix == '%'

# Create a FileCheck variable name based on an IR name.
def get_value_name(var, match):
  if var.isdigit():
    var = get_nameless_check_prefix_from_ir_value_match(match) + var
  var = var.replace('.', '_')
  var = var.replace('-', '_')
  return var.upper()

# Create a FileCheck variable from regex.
def get_value_definition(var, match):
  return '[[' + get_value_name(var, match) + ':' + get_ir_prefix_from_ir_value_match(match) + '.*]]'

# Use a FileCheck variable.
def get_value_use(var, match):
  return '[[' + get_value_name(var, match) + ']]'

# Replace IR value defs and uses with FileCheck variables.
def genericize_check_lines(lines, is_analyze, vars_seen, global_vars_seen):
  # This gets called for each match that occurs in
  # a line. We transform variables we haven't seen
  # into defs, and variables we have seen into uses.
  def transform_line_vars(match):
    pre = get_ir_prefix_from_ir_value_match(match)
    var = get_name_from_ir_value_match(match)
    for nameless_value in nameless_values:
        if re.match(r'^' + nameless_value.check_prefix + r'[0-9]+?$', var, re.IGNORECASE):
            warn("Change IR value name '%s' to prevent possible conflict with scripted FileCheck name." % (var,))
    if (pre, var) in vars_seen or (pre, var) in global_vars_seen:
      rv = get_value_use(var, match)
    else:
      if is_local_ir_value_match(match):
         vars_seen.add((pre, var))
      else:
         global_vars_seen.add((pre, var))
      rv = get_value_definition(var, match)
    # re.sub replaces the entire regex match
    # with whatever you return, so we have
    # to make sure to hand it back everything
    # including the commas and spaces.
    return match.group(1) + rv + match.group(match.lastindex)

  lines_with_def = []

  for i, line in enumerate(lines):
    # An IR variable named '%.' matches the FileCheck regex string.
    line = line.replace('%.', '%dot')
    # Ignore any comments, since the check lines will too.
    scrubbed_line = SCRUB_IR_COMMENT_RE.sub(r'', line)
    lines[i] = scrubbed_line
    if not is_analyze:
      # It can happen that two matches are back-to-back and for some reason sub
      # will not replace both of them. For now we work around this by
      # substituting until there is no more match.
      changed = True
      while changed:
          (lines[i], changed) = IR_VALUE_RE.subn(transform_line_vars, lines[i], count=1)
  return lines


def add_checks(output_lines, comment_marker, prefix_list, func_dict, func_name, check_label_format, is_asm, is_analyze, global_vars_seen_dict):
  # prefix_exclusions are prefixes we cannot use to print the function because it doesn't exist in run lines that use these prefixes as well.
  prefix_exclusions = set()
  printed_prefixes = []
  for p in prefix_list:
    checkprefixes = p[0]
    # If not all checkprefixes of this run line produced the function we cannot check for it as it does not
    # exist for this run line. A subset of the check prefixes might know about the function but only because
    # other run lines created it.
    if any(map(lambda checkprefix: func_name not in func_dict[checkprefix], checkprefixes)):
        prefix_exclusions |= set(checkprefixes)
        continue

  # prefix_exclusions is constructed, we can now emit the output
  for p in prefix_list:
    checkprefixes = p[0]
    for checkprefix in checkprefixes:
      if checkprefix in printed_prefixes:
        break

      # Check if the prefix is excluded.
      if checkprefix in prefix_exclusions:
        continue

      # If we do not have output for this prefix we skip it.
      if not func_dict[checkprefix][func_name]:
        continue

      # Add some space between different check prefixes, but not after the last
      # check line (before the test code).
      if is_asm:
        if len(printed_prefixes) != 0:
          output_lines.append(comment_marker)

      if checkprefix not in global_vars_seen_dict:
          global_vars_seen_dict[checkprefix] = set()
      global_vars_seen = global_vars_seen_dict[checkprefix]

      vars_seen = set()
      printed_prefixes.append(checkprefix)
      attrs = str(func_dict[checkprefix][func_name].attrs)
      attrs = '' if attrs == 'None' else attrs
      if attrs:
        output_lines.append('%s %s: Function Attrs: %s' % (comment_marker, checkprefix, attrs))
      args_and_sig = str(func_dict[checkprefix][func_name].args_and_sig)
      args_and_sig = genericize_check_lines([args_and_sig], is_analyze, vars_seen, global_vars_seen)[0]
      if '[[' in args_and_sig:
        output_lines.append(check_label_format % (checkprefix, func_name, ''))
        output_lines.append('%s %s-SAME: %s' % (comment_marker, checkprefix, args_and_sig))
      else:
        output_lines.append(check_label_format % (checkprefix, func_name, args_and_sig))
      func_body = str(func_dict[checkprefix][func_name]).splitlines()

      # For ASM output, just emit the check lines.
      if is_asm:
        output_lines.append('%s %s:       %s' % (comment_marker, checkprefix, func_body[0]))
        for func_line in func_body[1:]:
          if func_line.strip() == '':
            output_lines.append('%s %s-EMPTY:' % (comment_marker, checkprefix))
          else:
            output_lines.append('%s %s-NEXT:  %s' % (comment_marker, checkprefix, func_line))
        break

      # For IR output, change all defs to FileCheck variables, so we're immune
      # to variable naming fashions.
      func_body = genericize_check_lines(func_body, is_analyze, vars_seen, global_vars_seen)

      # This could be selectively enabled with an optional invocation argument.
      # Disabled for now: better to check everything. Be safe rather than sorry.

      # Handle the first line of the function body as a special case because
      # it's often just noise (a useless asm comment or entry label).
      #if func_body[0].startswith("#") or func_body[0].startswith("entry:"):
      #  is_blank_line = True
      #else:
      #  output_lines.append('%s %s:       %s' % (comment_marker, checkprefix, func_body[0]))
      #  is_blank_line = False

      is_blank_line = False

      for func_line in func_body:
        if func_line.strip() == '':
          is_blank_line = True
          continue
        # Do not waste time checking IR comments.
        func_line = SCRUB_IR_COMMENT_RE.sub(r'', func_line)

        # Skip blank lines instead of checking them.
        if is_blank_line:
          output_lines.append('{} {}:       {}'.format(
              comment_marker, checkprefix, func_line))
        else:
          output_lines.append('{} {}-NEXT:  {}'.format(
              comment_marker, checkprefix, func_line))
        is_blank_line = False

      # Add space between different check prefixes and also before the first
      # line of code in the test function.
      output_lines.append(comment_marker)
      break

def add_ir_checks(output_lines, comment_marker, prefix_list, func_dict,
                  func_name, preserve_names, function_sig, global_vars_seen_dict):
  # Label format is based on IR string.
  function_def_regex = 'define {{[^@]+}}' if function_sig else ''
  check_label_format = '{} %s-LABEL: {}@%s%s'.format(comment_marker, function_def_regex)
  add_checks(output_lines, comment_marker, prefix_list, func_dict, func_name,
             check_label_format, False, preserve_names, global_vars_seen_dict)

def add_analyze_checks(output_lines, comment_marker, prefix_list, func_dict, func_name):
  check_label_format = '{} %s-LABEL: \'%s%s\''.format(comment_marker)
  global_vars_seen_dict = {}
  add_checks(output_lines, comment_marker, prefix_list, func_dict, func_name,
             check_label_format, False, True, global_vars_seen_dict)


def check_prefix(prefix):
  if not PREFIX_RE.match(prefix):
        hint = ""
        if ',' in prefix:
          hint = " Did you mean '--check-prefixes=" + prefix + "'?"
        warn(("Supplied prefix '%s' is invalid. Prefix must contain only alphanumeric characters, hyphens and underscores." + hint) %
             (prefix))


def verify_filecheck_prefixes(fc_cmd):
  fc_cmd_parts = fc_cmd.split()
  for part in fc_cmd_parts:
    if "check-prefix=" in part:
      prefix = part.split('=', 1)[1]
      check_prefix(prefix)
    elif "check-prefixes=" in part:
      prefixes = part.split('=', 1)[1].split(',')
      for prefix in prefixes:
        check_prefix(prefix)
        if prefixes.count(prefix) > 1:
          warn("Supplied prefix '%s' is not unique in the prefix list." % (prefix,))


def get_autogennote_suffix(parser, args):
  autogenerated_note_args = ''
  for action in parser._actions:
    if not hasattr(args, action.dest):
      continue  # Ignore options such as --help that aren't included in args
    # Ignore parameters such as paths to the binary or the list of tests
    if action.dest in ('tests', 'update_only', 'opt_binary', 'llc_binary',
                       'clang', 'opt', 'llvm_bin', 'verbose'):
      continue
    value = getattr(args, action.dest)
    if action.const is not None:  # action stores a constant (usually True/False)
      # Skip actions with different constant values (this happens with boolean
      # --foo/--no-foo options)
      if value != action.const:
        continue
    if parser.get_default(action.dest) == value:
      continue  # Don't add default values
    autogenerated_note_args += action.option_strings[0] + ' '
    if action.const is None:  # action takes a parameter
      autogenerated_note_args += '%s ' % value
  if autogenerated_note_args:
    autogenerated_note_args = ' %s %s' % (UTC_ARGS_KEY, autogenerated_note_args[:-1])
  return autogenerated_note_args


def check_for_command(line, parser, args, argv, argparse_callback):
    cmd_m = UTC_ARGS_CMD.match(line)
    if cmd_m:
        cmd = cmd_m.group('cmd').strip().split(' ')
        argv = argv + cmd
        args = parser.parse_args(filter(lambda arg: arg not in args.tests, argv))
        if argparse_callback is not None:
          argparse_callback(args)
    return args, argv
