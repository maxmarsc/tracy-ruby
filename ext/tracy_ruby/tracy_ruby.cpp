#include <ruby.h>
#include <ruby/vm.h>
#include <cstring>
#include <tracy/TracyC.h>

#include "tracy_ruby.h"

namespace {

VALUE rb_startup(VALUE /*self*/) {
#ifdef TRACY_ENABLE
  ___tracy_startup_profiler();
#endif
  return Qnil;
}

VALUE rb_shutdown(VALUE /*self*/) {
#ifdef TRACY_ENABLE
  ___tracy_shutdown_profiler();
  return Qnil;
#endif
}

VALUE rb_started(VALUE /*self*/) {
#ifdef TRACY_ENABLE
  return static_cast<bool>(___tracy_profiler_started()) ? Qtrue : Qfalse;
#else
  return Qfalse;
#endif
}

struct ZoneGuard {
  TracyCZoneCtx ctx;
  int emitted;
};

VALUE rb_zoneBody(VALUE /*arg*/) {
  return rb_yield(Qnil);
}

VALUE rb_zoneEnsure(VALUE arg) {
#ifdef TRACY_ENABLE
  //NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
  auto* guard = reinterpret_cast<ZoneGuard*>(arg);
  if (guard->emitted != 0) {
    TracyCZoneEnd(guard->ctx);
  }
#endif
  return Qnil;
}

VALUE rb_zone(VALUE /* self*/, VALUE name) {
  rb_need_block();

#ifdef TRACY_ENABLE
  ZoneGuard guard{};
  guard.emitted = 0;

  if (static_cast<bool>(___tracy_profiler_started())) {
    // name
    const char* nptr = StringValueCStr(name);
    auto nlen        = static_cast<size_t>(RSTRING_LEN(name));

    // file & line
    auto line        = static_cast<uint32_t>(rb_sourceline());
    const char* file = rb_sourcefile();
    if (file == nullptr) {
      file = "(ruby)";
      line = 0;
    }

    // function
    const char* func = "(toplevel)";
    auto callee      = rb_frame_callee();
    if (callee != 0) {
      func = rb_id2name(callee);
    }

    auto srcloc = ___tracy_alloc_srcloc_name(line, file, std::strlen(file), func,
                                             std::strlen(func), nptr, nlen, 0);

    guard.ctx     = ___tracy_emit_zone_begin_alloc(srcloc, 1);
    guard.emitted = 1;
  }

  return rb_ensure(rb_zoneBody, Qnil, rb_zoneEnsure,
    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                   reinterpret_cast<VALUE>(&guard));
#else
  (void)name;
  return rb_yield(Qnil);
#endif
}

VALUE rb_frameMark(VALUE /*self*/) {
#ifdef TRACY_ENABLE
  ___tracy_emit_frame_mark(nullptr);
#endif
  return Qnil;
}

VALUE rb_message(VALUE /*self*/, VALUE message) {
#ifdef TRACY_ENABLE
  const char* msg = StringValueCStr(message);
  auto len        = static_cast<size_t>(RSTRING_LEN(message));
  ___tracy_emit_message(msg, len, 0);
#endif
  return Qnil;
}

}  // namespace

extern "C" {
  void Init_tracy_ruby() {
    VALUE mTracy = rb_define_module("Tracy");
    rb_define_singleton_method(mTracy, "startup", rb_startup, 0);
    rb_define_singleton_method(mTracy, "shutdown", rb_shutdown, 0);
    rb_define_singleton_method(mTracy, "started?", rb_started, 0);
    rb_define_singleton_method(mTracy, "zone", rb_zone, 1);
    rb_define_singleton_method(mTracy, "frame_mark", rb_frameMark, 0);
    rb_define_singleton_method(mTracy, "message", rb_message, 1);
  }
}