#ifndef BUILD_NODEJS
#error "Node module build would not work without defining BUILD_NODEJS"
#endif // BUILD_NODEJS

#include <napi.h>

#include "parser.h"
#include "response.h"
#include "rollback.h"

template <typename T>
auto execute (Napi::Env env, const T &cmd) -> Napi::Value {
  auto res = ticket::command::dispatch(cmd);
  if (auto err = res.error()) {
    auto error = Napi::Error::New(env, err->what());
    error.ThrowAsJavaScriptException();
    return {};
  }
  Napi::Value value;
  res.result().visit([&value, &env] (const auto &res) {
    value = ticket::response::toJsObject(env, res);
  });
  return value;
}

auto handler (const Napi::CallbackInfo &info) -> Napi::Value {
  try {
    auto args = info[0].As<Napi::Array>();
    ticket::Vector<std::string> converted;
    for (int i = 0; i < args.Length(); ++i) {
      converted.push_back(Napi::Value(args[i]).As<Napi::String>().Utf8Value());
    }
    ticket::Vector<std::string_view> view;
    for (const auto &str : converted) view.push_back(str);
    auto cmd = ticket::command::parse(view);
    if (auto err = cmd.error()) {
      auto error = Napi::Error::New(info.Env(), err->what());
      error.ThrowAsJavaScriptException();
      return {};
    }
    Napi::Value res;
    cmd.result().visit([&info, &res] (const auto &cmd) {
      res = execute(info.Env(), cmd);
    });
    return res;
  } catch (const ticket::Exception &e) {
    auto error = Napi::Error::New(info.Env(), e.what());
    error.ThrowAsJavaScriptException();
    return {};
  }
}

auto init (Napi::Env env, Napi::Object exports) -> Napi::Object {
  return Napi::Function::New(env, handler);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init)
