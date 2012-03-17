Emulation of *finally* in and using C++11.
================================

The code is a result of me hacking and exploring C++11. It is by no means promoting using of *finally* in C++. RAII is a better route most of the time, if not always.

It is very close to native try/catch in syntax, semantics and effects, with a few exceptions:

* There is no support for premature return. I certainly would like to work on it at a later time.
* It has to end with a semicolon. See example code below.
* Added support of *finally*, obviously.

It's a header only library, and has been compiled and mildly tested on *gcc 4.5* only. To compile, for example using my test program finally.cpp, you do:
<pre>
$ g++-4.5 -std=c++0x finally.cpp
</pre>


Have fun!

<pre>
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
    log.debug &lt;&lt; "exception caught: " &lt;&lt; x.what();
    if(timed_out)
    {
      log.error &lt;&lt; "timed out, throwing timeout_error.";
      throw timeout_error(x);
    }

    log.debug &lt;&lt; "handling exception";
    // error handling code
  }
  catchall
  {
    log.error &lt;&lt; "unknown exception, propagating caught exception.";
    throw;
  }
  finally
  {
    close(fd);
  };  // IMPORTANT: the ending semicolon is mandatory.

  log.info &lt;&lt; "exiting the function maturely";
}
</pre>

One more thing worth nothing, arguably subtle but potentially significant at times depending on the context/application. Unlike native try/catch, this mechanism allocates memory from heap (directly via *new*, indirectly via std:function<>::function and std:function<>::operator=) when establishing harness for user try/catch/finally clauses, but not while it is executing them.

Though the mechanism provides strong exception-safety, it is not no-throw. [1]

This implies, for maximum exception-safety, one may opt native try/catch in the outmost layer of exception handling, to handle/capture failures on the mechanism itself, e.g. main() and thread entry-points.

  [1]: http://en.wikipedia.org/wiki/Exception_guarantees        "Exception guarantees"