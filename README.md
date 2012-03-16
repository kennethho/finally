Emulation of *finally* in and using C++11.
================================

The code is a result of me hacking and exploring C++11. It is certainly not to promote using of *finally* in C++. RAII is a better route most of the time, if not always.

Its syntax is very close to, and as far as I can tell semantically equivalent to, native try/catch, with a few exceptions:

* There is no support for premature return. I certainly would like to work on it at a later time.
* It has to end with a semicolon. See example code below.
* Added support of finally clause, obviously.

It's a header only library. To compile, for example using my test program finally.cpp, you do:
<pre>
  $ g++-4.5 -std=c++0x finally.cpp
</pre>

It has been compiled and briefly tested on *gcc 4.5* only. Have fun!

<pre>
void finally_example()
{
  int fd = open(...);
  assert(fd != 0);

  try_
  {
    // some code
  }
  catch_(network_error& x)
  {
    log.error &lt;&lt; x.what();

    // error handling code
  }
  catchall
  {
    log.error &lt;&lt; "irrecoverable exception";
    throw;
  }
  finally
  {
    close(fd);
  };  // IMPORTANT: the ending semicolon is mandatory.

  log.info &lt;&lt; "exiting the function maturely";
}
</pre>

One more thing, arguably subtle but possibly significant at times depending on the context/application, worth nothing. Unlike native try/catch, this mechanism allocates memory from heap (directly via *new*, and indirectly via std:function<>::function). Though it provides strong exception-safety, it is not no-throw. [1]

This implies, for maximum exception-safety, one may opt native try/catch in the outmost layer of exception handling, to handle/capture failures on the mechanism itself, e.g. main() and thread entry-points.

  [1]: http://en.wikipedia.org/wiki/Exception_guarantees        "Exception guarantees"