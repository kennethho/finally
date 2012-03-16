Emulation of *finally* in and using C++11.
================================

The code is a result of me hacking and exploring C++11. It is certainly not to promote use of *finally* in C++. RAII is a better route most of the time, if not always.

Its sytax is very close to, and as far as I can tell semantically equivalent to, native try/catch, with a few exceptions:

* There is no support for premature return. I certainly would like to work on it at a later time.
* It has to end with a semicolon. See example code below.
* Added support of finally clause, obviously.

It's a header only library. To compile, for example using finally.cpp (my test program), you do:
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