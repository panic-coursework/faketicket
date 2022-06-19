const yaml = require('yaml')
const { readFileSync, writeFileSync } = require('fs')

const filenameBase = 'parser'
const filenameCpp = `${filenameBase}.cpp`
const filenameH = `${filenameBase}.h`
const filenameRunH = 'run.h'
const filenameNodeCpp = 'node.cpp'
const filenameDts = 'index.d.ts'

const commands = yaml.parse(readFileSync('commands.yml').toString())

const className = snake => snake.split('_').map(x => x.slice(0, 1).toUpperCase() + x.slice(1)).join('')
String.prototype.indent = function (level) {
  return this.split('\n').map(line => (' '.repeat(level * 2) + line).trimEnd()).join('\n')
}

const stdtype = str => str === 'string' ? 'std::string' : str
const nodestd = str => {
  const map = {
    int: 'number',
    bool: 'boolean',
    char: 'string',
    Date: 'DateString',
  }
  return map[str] ?? str
}
const type = (arg, std = stdtype(arg.type)) => arg.array
  ? `Vector<${std}>`
  : arg.optional
    ? `Optional<${std}>`
    : std
const nodeType = (arg, std = nodestd(arg.type)) => arg.array ? `${std}[]` : std
const getValue = (type, varname) => {
  if (type === 'int') return `atoi(${varname})`
  if (type === 'bool') return `${varname}[0] == 't'`
  if (type === 'char') return `*${varname}`
  if (type === 'SortType') return `${varname}[0] == 't' ? kTime : kCost`
  if (type === 'Duration') return `Duration(atoi(${varname}))`
  if ([ 'Date', 'Instant' ].includes(type)) return `${type}(${varname})`
  return varname
}
const getNodeValue = (type, varname) => {
  if (type === 'int') return `CPP_INT(${varname})`
  if (type === 'bool') return `CPP_BOOL(${varname})`
  if (type === 'char') return `CPP_STR(${varname})[0]`
  if (type === 'SortType') return `CPP_STR(${varname})[0] == 't' ? kTime : kCost`
  if (type === 'Duration') return `Duration(CPP_INT(${varname}))`
  if ([ 'Date', 'Instant' ].includes(type)) return `${type}(CPP_STR(${varname}).data())`
  return `CPP_STR(${varname})`
}

const ARG_RE = /^(?<type>[^ ]+) (?<name>[a-zA-Z0-9]+)(?<optional>\?)?(?<array>\[\])?(?<default> = .+)?$/
const parseArg = str => str.match(ARG_RE).groups
const declareArg = ([ _, value ]) => {
  const parsed = parseArg(value)
  return `${type(parsed)} ${parsed.name}${parsed.default ?? ''};`
}
const declareNodeArg = ([ _, value ]) => {
  const parsed = parseArg(value)
  return `${parsed.name}${parsed.optional || parsed.default ? '?' : ''}: ${nodeType(parsed)}`
}
const getArray = (name, parsed, varname) => {
  if (parsed.type === 'string') return `std::string val = ${varname};
  res.${name} = copyStrings(split(val, '|'));`
  return `std::string val = ${varname};
  auto values = split(val, '|');
  res.${name}.reserve(values.size());
  for (auto &str : values) {
    res.${name}.push_back(${getValue(parsed.type, 'str.data()')});
  }`
}
const getNodeArray = (name, parsed, varname) => `
{
  auto array = ${varname}.As<Napi::Array>();
  cmd.${name}.reserve(array.Length());
  for (int i = 0; i < array.Length(); ++i) {
    cmd.${name}.push_back(${getNodeValue(parsed.type, 'array.Get(i)')});
  }
}
`.trim()
const testArg = ([ name, value ]) => {
  const parsed = parseArg(value)
  return `
if (arg == "${name}") {
  ${parsed.array
    ? getArray(parsed.name, parsed, 'argv[++i].data()')
    : `res.${parsed.name} = ${getValue(parsed.type, 'argv[++i].data()')};`}
} else
`.slice(1, -1)
}
const nodeArg = ([ _, value ]) => {
  const parsed = parseArg(value)
  if (parsed.array) return getNodeArray(parsed.name, parsed, `args.Get("${parsed.name}")`)
  return `
${parsed.optional || parsed.default ? `if (!isNullish(args.Get("${parsed.name}"))) ` : ''}cmd.${parsed.name} = ${getNodeValue(parsed.type, `args.Get("${parsed.name}")`)};
`.trim()
}
const header = ([ name, args ]) => `
struct ${className(name)} {
  ${Object.entries(args).map(declareArg).join('\n  ')}
};
`.slice(1)
const nodeInterface = ([ name, args ]) => Object.keys(args).length === 0 ? '' : `
interface ${className(name)}Options {
${Object.entries(args).map(declareNodeArg).join('\n').indent(1)}
}
`.slice(1)

const implementation = ([ name, args ]) => `
if (argv0 == "${name}") {
${Object.keys(args).length == 0 ? `
  return Command(${className(name)}());`.slice(1) : `
  ${className(name)} res;
  for (int i = 1; i < argv.size(); ++i) {
    auto &arg = argv[i];
${Object.entries(args).map(testArg).join(' ').indent(2)} {
      return ParseException();
    }
  }
  return res;
`.slice(1, -1)}
} else
`.trim()
const nodeImplementation = ([ name, args ]) => `
auto node${className(name)} (const Napi::CallbackInfo &info)
  -> Napi::Value {
  ${className(name)} cmd;
  ${Object.keys(args).length == 0 ? '' : `
  auto args = info[0].ToObject();
${Object.entries(args).map(nodeArg).join('\n').indent(1)}
  `.trimStart()}return handleCommand(info.Env(), cmd);
}
`.trim()

const runOverload = name => `auto run (const ${name} &cmd) -> Result<Response, Exception>;`
const nodeDeclaration = ([ name, value ]) => `export function ${className(name).slice(0, 1).toLowerCase()}${className(name).slice(1)} (${Object.keys(value).length === 0 ? '' : `options: ${className(name)}Options`}): Response`

const createExport = name => `exports["${name.slice(0, 1).toLowerCase()}${name.slice(1)}"] = Napi::Function::New(env, node${name});`

const defineGuard = 'TICKET_PARSER_H_'
const defineGuardRun = 'TICKET_RUN_H_'
const ns = `ticket::command`
// this file (command-parser.js) is not autogenerated.
const autogenDisclaimer = '// This file is autogenerated. Do not modify.'

const runH = `
${autogenDisclaimer}

#ifndef ${defineGuardRun}
#define ${defineGuardRun}

#include "parser.h"
#include "result.h"
#include "response.h"

namespace ${ns} {

/**
 * @brief Visitor for the commands.
 *
 * The main function uses this visitor after parsing a
 * command, to actually dispatch it. Overloads of operator()
 * are callbacks of the commands.
 *
 * The implementations are in the corresponding source
 * files, not in run.cpp.
 */
${Object.keys(commands).map(className).map(runOverload).join('\n')}

} // namespace ${ns}

#endif // ${defineGuardRun}
`.slice(1)

const h = `
${autogenDisclaimer}

#ifndef ${defineGuard}
#define ${defineGuard}

#include <iostream>

#include "datetime.h"
#include "exception.h"
#include "optional.h"
#include "variant.h"
#include "result.h"

/// Classes and parsers for commands.
namespace ${ns} {

enum SortType { kTime, kCost };

${Object.entries(commands).map(header).join('\n')}

using Command = Variant<
  ${Object.keys(commands).map(className).join(',\n  ')}
>;

/**
 * @brief parses the command stored in str.
 *
 * this function is autogenerated.
 */
auto parse (std::string &str)
  -> Result<Command, ParseException>;
auto parse (const Vector<std::string_view> &argv)
  -> Result<Command, ParseException>;

} // namespace ${ns}

#endif // ${defineGuard}
`.slice(1)

const cpp = `
${autogenDisclaimer}

#include "${filenameH}"

#include "utility.h"

namespace ${ns} {

auto parse (std::string &str)
  -> Result<Command, ParseException> {
  auto argv = split(str, ' ');
  return parse(argv);
}

auto parse (const Vector<std::string_view> &argv)
  -> Result<Command, ParseException> {
  auto &argv0 = argv[0];
${Object.entries(commands).map(implementation).join(' ').indent(1)} {
    return ParseException();
  }
}

} // namespace ${ns}
`.slice(1)
const nodeCpp = `
${autogenDisclaimer}

#ifndef BUILD_NODEJS
#error "This file only works in Node builds"
#endif // BUILD_NODEJS

#include <napi.h>

#include "exception.h"
#include "parser.h"
#include "response.h"
#include "result.h"
#include "run.h"
#include "vector.h"

namespace ${ns} {

inline auto isNullish (Napi::Value value) -> bool {
  return value.IsNull() || value.IsUndefined();
}

template <typename Cmd>
inline auto handleCommand (Napi::Env env, const Cmd &cmd)
  -> Napi::Value {
  try {
    auto resp = run(cmd);
    if (auto err = resp.error()) {
      auto error = Napi::Error::New(env, err->what());
      error.ThrowAsJavaScriptException();
      return {};
    }
    Napi::Value res;
    resp.result().visit([&env, &res] (const auto &resp) {
      res = response::toJsObject(env, resp);
    });

    return res;
  } catch (const Exception &e) {
    auto error = Napi::Error::New(env, e.what());
    error.ThrowAsJavaScriptException();
    return {};
  }
}

#define CPP_STR(x) ((x).As<Napi::String>().Utf8Value())
#define CPP_INT(x) ((x).As<Napi::Number>().Int32Value())
#define CPP_BOOL(x) ((x).As<Napi::Boolean>().Value())

${Object.entries(commands).map(nodeImplementation).join('\n\n')}

#undef CPP_STR
#undef CPP_INT
#undef CPP_BOOL

auto init (Napi::Env env, Napi::Object exports)
 -> Napi::Object {
${Object.keys(commands).map(className).map(createExport).join('\n').indent(1)}
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init)

} // namespace ${ns}
`.slice(1)

const dts = `
${autogenDisclaimer}

import { Response } from './response'

export type SortType = 'time' | 'cost'
export type Duration = number

type Numeral = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
type NumberSegment = \`\${Numeral}\${Numeral}\`
export type DateString = \`\${NumberSegment}-\${NumberSegment}\`
export type Instant = \`\${NumberSegment}:\${NumberSegment}\`

${Object.entries(commands).map(nodeInterface).join('')}
${Object.entries(commands).map(nodeDeclaration).join('\n')}
`.slice(1)

const dir = 'src/'
writeFileSync(dir + filenameH, h)
writeFileSync(dir + filenameRunH, runH)
writeFileSync(dir + filenameCpp, cpp)
writeFileSync(dir + filenameNodeCpp, nodeCpp)
writeFileSync(filenameDts, dts)
