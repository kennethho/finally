#ifndef FINALLY_HPP
#define FINALLY_HPP

/*
Copyright (c) 2011 Kenneth Ho https://github.com/kennethho

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <functional>
#include <memory>

#if(0)

void finally_example()
{
  int fd = open(...);
  assert(fd != 0);

  try_
  {
    // some code
  }
  catch_(domain_error& x)
  {
    log.debug << "exception caught: " << x.what();
    if(timed_out)
    {
      log.error << "timed out, throwing timeout_error.";
      throw timeout_error(x);
    }

    log.debug << "handling exception";
    // error handling code
  }
  catchall
  {
    log.error << "unknown exception, propagating caught exception.";
    throw;
  }
  finally
  {
    close(fd);
  };  // IMPORTANT: the ending semicolon is mandatory.

  log.info << "exiting the function maturely";
}

#endif

#define try_ \
  detail::try_tag() << [&]()

#define catch_(x) \
  << [&](x)

#define catchall \
  << [&]()

#define finally \
  << detail::finally_tag() << [&]()

namespace detail
{

  struct try_tag {};
  struct finally_tag {};

  template<typename LambdaExpr>
  struct lambda_traits
  {
    template<typename ReturnType, typename... Args>
    static ReturnType return_type_helper(ReturnType (LambdaExpr::*)(Args...) const);
    template<typename ReturnType, typename... Args>
    static std::tuple<Args...> args_helper(ReturnType (LambdaExpr::*)(Args...) const);
    template <class ReturnType, class ...Args>
    static std::function<ReturnType (Args...)> function_helper(ReturnType*, std::tuple<Args...>*);

    // return type
    typedef decltype(
      return_type_helper(&LambdaExpr::operator()) 
      ) return_type;
    // arguments' types in a std::tuple
    typedef decltype( 
      args_helper(&LambdaExpr::operator()) 
      ) args_tuple;
    // std::function<R (Args...)> matching the lambda expression
    typedef decltype(
      function_helper(
        (return_type*)0,
        (args_tuple*)0 )
      ) function_type;
  };

  struct try_block
  {
    std::function<void ()> try_clause_;
    try_block() {}
    explicit try_block(std::function<void ()> try_clause)
      : try_clause_(std::move(try_clause)) {}
  };
  try_block operator<<(try_tag, std::function<void ()> try_clause)
  {
    return try_block(std::move(try_clause));
  }

  struct exception_state_t
  {
    typedef int value_type;

    // no exception, i.e. program in normal execution flow
    static const value_type not_applicable = 0;
    // exception thrown from try clause, and is in the process
    // of matching a catch clause.
    static const value_type propagating = 1;
    // excpetion thrown from try clause, and had been caught 
    // and rethrown by a catch clause. stop matching caught clauses.
    static const value_type rethrown = 2;
  };

  struct try_catch_assembly
  {
    struct context
    {
      exception_state_t::value_type exception_state_;
      std::function<void ()> stack_;

      context()
        : exception_state_(exception_state_t::not_applicable)
      {}
    };
    std::unique_ptr<context> cntx_;

    try_catch_assembly(const try_catch_assembly& other) = delete;
    try_catch_assembly(try_catch_assembly&& other) = default;

    template <class T>
    try_catch_assembly(
      std::function<void ()> try_clause, 
      std::function<void (T)> catch_clause)
      : cntx_(new context())
    {
      context* const cntx = cntx_.get();

      cntx->stack_ = [try_clause, catch_clause, cntx]() {
        try {
          cntx->exception_state_ = exception_state_t::propagating;
          try_clause();
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
        catch(T x) {
          cntx->exception_state_ = exception_state_t::rethrown;
          catch_clause(x);
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
      };
    }
    try_catch_assembly(
      std::function<void ()> try_clause, 
      std::function<void ()> catchall_clause)
      : cntx_(new context())
    {
      context* const cntx = cntx_.get();

      cntx->stack_ = [try_clause, catchall_clause, cntx]() {
        try {
          cntx->exception_state_ = exception_state_t::propagating;
          try_clause();
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
        catch(...) {
          cntx->exception_state_ = exception_state_t::rethrown;
          catchall_clause();
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
      };
    }

    template <class T>
    try_catch_assembly(
      try_catch_assembly try_catch, 
      std::function<void (T)> catch_clause)
      : cntx_(move(try_catch.cntx_))
    {
      context* const cntx = cntx_.get();

      std::function<void ()> old_stack(move(cntx->stack_));
      cntx->stack_ = [old_stack, catch_clause, cntx]() {
        try {
          old_stack();
        }
        catch(T x) {
          if(cntx->exception_state_ == exception_state_t::not_applicable)
            return;
          if(cntx->exception_state_ == exception_state_t::rethrown)
            throw;

          cntx->exception_state_ = exception_state_t::rethrown;
          catch_clause(x);
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
      };
    }
    try_catch_assembly(
      try_catch_assembly try_catch, 
      std::function<void ()> catchall_clause)
      : cntx_(move(try_catch.cntx_))
    {
      context* const cntx = cntx_.get();

      std::function<void ()> old_stack(move(cntx->stack_));
      cntx->stack_ = [old_stack, catchall_clause, cntx]() {
        try {
          old_stack();
        }
        catch(...) {
          if(cntx->exception_state_ == exception_state_t::rethrown)
            throw;

          cntx->exception_state_ = exception_state_t::rethrown;
          catchall_clause();
          cntx->exception_state_ = exception_state_t::not_applicable;
        }
      };
    }

    ~try_catch_assembly() noexcept(false)
    {
      if(cntx_.get() == false)
        return;

      cntx_->stack_();
    }
  };

  template <class LambdaExpr>
  try_catch_assembly operator<<(try_block try_blk, LambdaExpr lambda_expr)
  {
    typename lambda_traits<LambdaExpr>::function_type catch_clause(lambda_expr);
    return try_catch_assembly(move(try_blk.try_clause_), std::move(catch_clause));
  }
  template <class LambdaExpr>
  try_catch_assembly operator<<(try_catch_assembly try_catch, LambdaExpr lambda_expr)
  {
    typename lambda_traits<LambdaExpr>::function_type catch_clause(lambda_expr);
    return try_catch_assembly(std::move(try_catch), std::move(catch_clause));
  }

  struct xpremature_try_catch_finally_assembly
  {
    std::unique_ptr<try_catch_assembly::context> try_catch_;

    explicit xpremature_try_catch_finally_assembly(try_catch_assembly try_catch)
      : try_catch_(move(try_catch.cntx_))
    {}
    xpremature_try_catch_finally_assembly(const xpremature_try_catch_finally_assembly&) = delete;
    xpremature_try_catch_finally_assembly(xpremature_try_catch_finally_assembly&& other)
      : try_catch_(move(other.try_catch_))
    {
    }
  };
  xpremature_try_catch_finally_assembly operator<<(try_catch_assembly try_catch, finally_tag)
  {
    return xpremature_try_catch_finally_assembly(std::move(try_catch));
  }

  struct try_catch_finally_assembly
  {
    std::unique_ptr<try_catch_assembly::context> try_catch_;
    std::function<void ()> finally_clause_;

    try_catch_finally_assembly(
      xpremature_try_catch_finally_assembly pre, 
      std::function<void ()> finally_clause)
      : try_catch_(move(pre.try_catch_)),
        finally_clause_(move(finally_clause))
    {
    }
    try_catch_finally_assembly(const try_catch_finally_assembly&) = delete;
    try_catch_finally_assembly(try_catch_finally_assembly&& other) = default;
    ~try_catch_finally_assembly() noexcept(false)
    {
      if(try_catch_.get() == false)
        return;

      try {
        try_catch_->stack_();
      }
      catch(...) {
        finally_clause_();
        throw;
      }
      finally_clause_();
    }
  };

  try_catch_finally_assembly operator<<(
    xpremature_try_catch_finally_assembly pre, 
    std::function<void ()> finally_clause)
  {
    return try_catch_finally_assembly(std::move(pre), std::move(finally_clause));
  }

} // namespace detail

#endif // FINALLY_HPP
