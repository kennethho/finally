#include <iostream>
#include <sstream>
#include <cassert>
#include <stdexcept>

#include "finally.hpp"

using namespace std;

void test_try_catchall_finally_1(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw 0;
    out << "c";
  }
  catchall
  {
    out << "d";
    throw;
  }
  finally
  {
    out << "e";
  };
  out << "f";
}
void test_try_catchall_finally_2(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw 0;
    out << "c";
  }
  catchall
  {
    out << "d";
  }
  finally
  {
    out << "e";
  };
  out << "f";
}
void test_try_catch_1(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw runtime_error("hello");
    out << "c";
  }
  catch_(exception& x)
  {
    out << "d";
  };
  out << "e";
}
void test_try_catch_2(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw runtime_error("");
    out << "c";
  }
  catch_(runtime_error& x)
  {
    out << "d";
  }
  catch_(exception& x)
  {
    out << "e";
  };
  out << "f";
}
void test_try_catch_3(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw exception();
    out << "c";
  }
  catch_(runtime_error& x)
  {
    out << "d";
  }
  catch_(exception& x)
  {
    out << "e";
  };
  out << "f";
}
void test_try_catch_4(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw exception();
    out << "c";
  }
  catch_(runtime_error& x)
  {
    out << "d";
  }
  catch_(exception& x)
  {
    out << "e";
    throw;
  };
  out << "f";
}

void test_try_catchall_1(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw exception();
    out << "c";
  }
  catch_(runtime_error& x)
  {
    out << "d";
  }
  catchall
  {
    out << "e";
  };
  out << "f";
}
void test_try_catchall_2(bool throws, ostream& out)
{
  out << "a";
  try_
  {
    out << "b";
    if(throws)
      throw exception();
    out << "c";
  }
  catch_(exception& x)
  {
    out << "d";
  }
  catchall
  {
    out << "e";
  };
  out << "f";
}

// helper to absorb exception from test()
template <class F>
string test(bool throws, bool should_caught, F f)
{
  bool exception_caught = false;
  ostringstream out;
  try
  {
    f(throws, out);
  }
  catch(...)
  {
    exception_caught = true;
  }
  string result = out.str();
  cout << result << endl;
  assert(exception_caught == should_caught);
  return result;
}

int main()
{
  string result;

  result = test(true, true, test_try_catchall_finally_1);
  assert(result == "abde");
  result = test(false, false, test_try_catchall_finally_1);
  assert(result == "abcef");

  result = test(true, false, test_try_catchall_finally_2);
  assert(result == "abdef");
  result = test(false, false, test_try_catchall_finally_2);
  assert(result == "abcef");

  result = test(true, false, test_try_catch_1);
  assert(result == "abde");
  result = test(false, false, test_try_catch_1);
  assert(result == "abce");

  result = test(true, false, test_try_catch_2);
  assert(result == "abdf");
  result = test(false, false, test_try_catch_2);
  assert(result == "abcf");

  result = test(true, false, test_try_catch_3);
  assert(result == "abef");
  result = test(false, false, test_try_catch_3);
  assert(result == "abcf");

  result = test(true, true, test_try_catch_4);
  assert(result == "abe");
  result = test(false, false, test_try_catch_4);
  assert(result == "abcf");

  result = test(true, false, test_try_catchall_1);
  assert(result == "abef");
  result = test(false, false, test_try_catchall_1);
  assert(result == "abcf");

  result = test(true, false, test_try_catchall_2);
  assert(result == "abdf");
  result = test(false, false, test_try_catchall_2);
  assert(result == "abcf");

  return 0;
}
