Emulation of (try, catch, catch-all and) finally in and using C++11.

The code is a result of me playing around with C++11. It is not to promote using "finally" in C++. RAII is a better route, at least most of the time.

Its sytax is very close to, and as far as I can tell compliant to semanteics of native try/catch(x)/catch(...), with one exception. There is no support for premature return, though I do want to make that work in the future.

It's a header only library. To compile, for example using finally.cpp (my test program), you do:
<pre>
  $ g++-4.5 -std=c++0x finally.cpp
</pre>


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
  };
  
  log.info &lt;&lt; "exiting the function maturely";
}
</pre>