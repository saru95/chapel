use UnitTest;

proc testAssert(x,reason="") {
  var sep = "=="*40;
  try {
    UnitTest.assertTrue(x);
  } catch e {
    writeln("Error Caught in "+reason);
    writeln(e);
    writeln(sep);
  }
}
proc main() {
  testAssert(true,"x == true");
  testAssert(false,"x == false");
  testAssert(1==2,"expresion '1==2'");
}