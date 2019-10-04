// -*- C++ -*-
//===----------------------------- coroutine -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_COROUTINE
#define _LIBCPP_EXPERIMENTAL_COROUTINE

/**
    experimental/coroutine synopsis

// C++next

namespace std {
namespace experimental {
inline namespace coroutines_v1 {

  // 18.11.1 coroutine traits
template <typename R, typename... ArgTypes>
class coroutine_traits;
// 18.11.2 coroutine handle
template <typename Promise = void>
class coroutine_handle;
// 18.11.2.7 comparison operators:
bool operator==(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator!=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
// 18.11.3 trivial awaitables
struct suspend_never;
struct suspend_always;
// 18.11.2.8 hash support:
template <class T> struct hash;
template <class P> struct hash<coroutine_handle<P>>;

} // namespace coroutines_v1
} // namespace experimental
} // namespace std

 */

#include <__config>
#include <new>
#include <type_traits>
#include <functional>
#include <memory> // for hash<T*>
#include <cstddef>
#include <cassert>
//#include <__debug>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

#ifdef _LIBCPP_HAS_NO_COROUTINES
# if defined(_LIBCPP_WARNING)
    _LIBCPP_WARNING("<experimental/coroutine> cannot be used with this compiler")
# else
#   warning <experimental/coroutine> cannot be used with this compiler
# endif
#endif

#ifndef _LIBCPP_HAS_NO_COROUTINES

_LIBCPP_BEGIN_NAMESPACE_EXPERIMENTAL_COROUTINES

template <class _Tp, class = void>
struct __coroutine_traits_sfinae {};

template <class>
struct __void_t { typedef void type; };

template <class _Tp>
struct __coroutine_traits_sfinae<
    _Tp, typename __void_t<typename _Tp::promise_type>::type>
{
  using promise_type = typename _Tp::promise_type;
};

#define _LIBCPP_TEMPLATE_VIS __attribute__((__type_visibility__("default")))
#ifndef _LIBCPP_INLINE_VISIBILITY
#define _LIBCPP_INLINE_VISIBILITY __attribute__ ((__visibility__("hidden"), __always_inline__))
#endif 
//#define _LIBCPP_CONSTEXPR constexpr
#define _LIBCPP_CONSTEXPR
#define _NOEXCEPT
#define _LIBCPP_ASSERT(x, m) ((void)0)
#define _LIBCPP_CONSTEXPR_AFTER_CXX17 constexpr
#define _LIBCPP_TYPE_VIS __attribute__ ((__visibility__("default")))
#define _LIBCPP_BEGIN_NAMESPACE_STD namespace std {
#define _LIBCPP_END_NAMESPACE_STD }
//#define _VSTD std::_LIBCPP_NAMESPACE
#define _VSTD std




template <typename _Ret, typename... _Args>
struct _LIBCPP_TEMPLATE_VIS coroutine_traits
    : public __coroutine_traits_sfinae<_Ret>
{
};

template <typename _Promise = void>
class _LIBCPP_TEMPLATE_VIS coroutine_handle;

template <>
class _LIBCPP_TEMPLATE_VIS coroutine_handle<void> {
public:
    _LIBCPP_INLINE_VISIBILITY
    _LIBCPP_CONSTEXPR coroutine_handle() _NOEXCEPT : __handle_(nullptr) {}

    _LIBCPP_INLINE_VISIBILITY
    _LIBCPP_CONSTEXPR coroutine_handle(nullptr_t) _NOEXCEPT : __handle_(nullptr) {}

    _LIBCPP_INLINE_VISIBILITY
    coroutine_handle& operator=(nullptr_t) _NOEXCEPT {
        __handle_ = nullptr;
        return *this;
    }

    _LIBCPP_INLINE_VISIBILITY
    _LIBCPP_CONSTEXPR void* address() const _NOEXCEPT { return __handle_; }

    _LIBCPP_INLINE_VISIBILITY
    _LIBCPP_CONSTEXPR explicit operator bool() const _NOEXCEPT { return __handle_; }

    _LIBCPP_INLINE_VISIBILITY
    void operator()() { resume(); }

    _LIBCPP_INLINE_VISIBILITY
    void resume() {
      _LIBCPP_ASSERT(__is_suspended(),
                     "resume() can only be called on suspended coroutines");
      _LIBCPP_ASSERT(!done(),
                "resume() has undefined behavior when the coroutine is done");
      __builtin_coro_resume(__handle_);
    }

    _LIBCPP_INLINE_VISIBILITY
    void destroy() {
      _LIBCPP_ASSERT(__is_suspended(),
                     "destroy() can only be called on suspended coroutines");
      __builtin_coro_destroy(__handle_);
    }

    _LIBCPP_INLINE_VISIBILITY
    bool done() const {
      _LIBCPP_ASSERT(__is_suspended(),
                     "done() can only be called on suspended coroutines");
      return __builtin_coro_done(__handle_);
    }

public:
    _LIBCPP_INLINE_VISIBILITY
    static coroutine_handle from_address(void* __addr) _NOEXCEPT {
        coroutine_handle __tmp;
        __tmp.__handle_ = __addr;
        return __tmp;
    }

    // FIXME: Should from_address(nullptr) be allowed?
    _LIBCPP_INLINE_VISIBILITY
    static coroutine_handle from_address(nullptr_t) _NOEXCEPT {
      return coroutine_handle(nullptr);
    }

    template <class _Tp, bool _CallIsValid = false>
    static coroutine_handle from_address(_Tp*) {
      static_assert(_CallIsValid,
       "coroutine_handle<void>::from_address cannot be called with "
        "non-void pointers");
    }

private:
  bool __is_suspended() const _NOEXCEPT  {
    // FIXME actually implement a check for if the coro is suspended.
    return __handle_;
  }

  template <class _PromiseT> friend class coroutine_handle;
  void* __handle_;
};

// 18.11.2.7 comparison operators:
inline _LIBCPP_INLINE_VISIBILITY
bool operator==(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return __x.address() == __y.address();
}
inline _LIBCPP_INLINE_VISIBILITY
bool operator!=(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return !(__x == __y);
}
inline _LIBCPP_INLINE_VISIBILITY
bool operator<(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return less<void*>()(__x.address(), __y.address());
}
inline _LIBCPP_INLINE_VISIBILITY
bool operator>(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return __y < __x;
}
inline _LIBCPP_INLINE_VISIBILITY
bool operator<=(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return !(__x > __y);
}
inline _LIBCPP_INLINE_VISIBILITY
bool operator>=(coroutine_handle<> __x, coroutine_handle<> __y) _NOEXCEPT {
    return !(__x < __y);
}

template <typename _Promise>
class _LIBCPP_TEMPLATE_VIS coroutine_handle : public coroutine_handle<> {
    using _Base = coroutine_handle<>;
public:
#ifndef _LIBCPP_CXX03_LANG
    // 18.11.2.1 construct/reset
    using coroutine_handle<>::coroutine_handle;
#else
    _LIBCPP_INLINE_VISIBILITY coroutine_handle() _NOEXCEPT : _Base() {}
    _LIBCPP_INLINE_VISIBILITY coroutine_handle(nullptr_t) _NOEXCEPT : _Base(nullptr) {}
#endif
    _LIBCPP_INLINE_VISIBILITY
    coroutine_handle& operator=(nullptr_t) _NOEXCEPT {
        _Base::operator=(nullptr);
        return *this;
    }

    _LIBCPP_INLINE_VISIBILITY
    _Promise& promise() const {
        return *static_cast<_Promise*>(
            __builtin_coro_promise(this->__handle_, __alignof(_Promise), false));
    }

public:
    _LIBCPP_INLINE_VISIBILITY
    static coroutine_handle from_address(void* __addr) _NOEXCEPT {
        coroutine_handle __tmp;
        __tmp.__handle_ = __addr;
        return __tmp;
    }

    // NOTE: this overload isn't required by the standard but is needed so
    // the deleted _Promise* overload doesn't make from_address(nullptr)
    // ambiguous.
    // FIXME: should from_address work with nullptr?
    _LIBCPP_INLINE_VISIBILITY
    static coroutine_handle from_address(nullptr_t) _NOEXCEPT {
      return coroutine_handle(nullptr);
    }

    template <class _Tp, bool _CallIsValid = false>
    static coroutine_handle from_address(_Tp*) {
      static_assert(_CallIsValid,
       "coroutine_handle<promise_type>::from_address cannot be called with "
        "non-void pointers");
    }

    template <bool _CallIsValid = false>
    static coroutine_handle from_address(_Promise*) {
      static_assert(_CallIsValid,
       "coroutine_handle<promise_type>::from_address cannot be used with "
        "pointers to the coroutine's promise type; use 'from_promise' instead");
    }

    _LIBCPP_INLINE_VISIBILITY
    static coroutine_handle from_promise(_Promise& __promise) _NOEXCEPT {
        typedef typename remove_cv<_Promise>::type _RawPromise;
        coroutine_handle __tmp;
        __tmp.__handle_ = __builtin_coro_promise(
            _VSTD::addressof(const_cast<_RawPromise&>(__promise)),
             __alignof(_Promise), true);
        return __tmp;
    }
};

#if __has_builtin(__builtin_coro_noop)
struct noop_coroutine_promise {};

template <>
class _LIBCPP_TEMPLATE_VIS coroutine_handle<noop_coroutine_promise>
    : public coroutine_handle<> {
  using _Base = coroutine_handle<>;
  using _Promise = noop_coroutine_promise;
public:

  _LIBCPP_INLINE_VISIBILITY
  _Promise& promise() const {
    return *static_cast<_Promise*>(
      __builtin_coro_promise(this->__handle_, __alignof(_Promise), false));
  }

  _LIBCPP_CONSTEXPR explicit operator bool() const _NOEXCEPT { return true; }
  _LIBCPP_CONSTEXPR bool done() const _NOEXCEPT { return false; }

  _LIBCPP_CONSTEXPR_AFTER_CXX17 void operator()() const _NOEXCEPT {}
  _LIBCPP_CONSTEXPR_AFTER_CXX17 void resume() const _NOEXCEPT {}
  _LIBCPP_CONSTEXPR_AFTER_CXX17 void destroy() const _NOEXCEPT {}

private:
  _LIBCPP_INLINE_VISIBILITY
  friend coroutine_handle<noop_coroutine_promise> noop_coroutine() _NOEXCEPT;

  _LIBCPP_INLINE_VISIBILITY coroutine_handle() _NOEXCEPT {
    this->__handle_ = __builtin_coro_noop();
  }
};

using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

inline _LIBCPP_INLINE_VISIBILITY
noop_coroutine_handle noop_coroutine() _NOEXCEPT {
  return noop_coroutine_handle();
}
#endif // __has_builtin(__builtin_coro_noop)

struct _LIBCPP_TYPE_VIS suspend_never {
  _LIBCPP_INLINE_VISIBILITY
  bool await_ready() const _NOEXCEPT { return true; }
  _LIBCPP_INLINE_VISIBILITY
  void await_suspend(coroutine_handle<>) const _NOEXCEPT {}
  _LIBCPP_INLINE_VISIBILITY
  void await_resume() const _NOEXCEPT {}
};

struct _LIBCPP_TYPE_VIS suspend_always {
  _LIBCPP_INLINE_VISIBILITY
  bool await_ready() const _NOEXCEPT { return false; }
  _LIBCPP_INLINE_VISIBILITY
  void await_suspend(coroutine_handle<>) const _NOEXCEPT {}
  _LIBCPP_INLINE_VISIBILITY
  void await_resume() const _NOEXCEPT {}
};

_LIBCPP_END_NAMESPACE_EXPERIMENTAL_COROUTINES

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Tp>
struct hash<_VSTD_CORO::coroutine_handle<_Tp> > {
    using __arg_type = _VSTD_CORO::coroutine_handle<_Tp>;
    _LIBCPP_INLINE_VISIBILITY
    size_t operator()(__arg_type const& __v) const _NOEXCEPT
    {return hash<void*>()(__v.address());}
};

_LIBCPP_END_NAMESPACE_STD

#endif // !defined(_LIBCPP_HAS_NO_COROUTINES)

#endif /* _LIBCPP_EXPERIMENTAL_COROUTINE */
