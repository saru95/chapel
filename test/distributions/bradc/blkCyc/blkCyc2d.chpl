use BlockCycDist;

config const m = 25, n = 35;

var MyBlkCyc = new BlockCyclic(rank=2, idxType=int(32), low=(1,0), blk=(4, 8));

writeln("Declaring D:");
var D: domain(2) distributed MyBlkCyc = [1..m, 0..n];
writeln();

writeln("Declaring D2:");
var D2: domain(2) distributed MyBlkCyc = [1..7, 0..0];
writeln();


writeln("D is: ", D);
writeln("D2 is: ", D2);

var A: [D] real;
var A2: [D2] real;

forall (i,j) in D do
  A(i,j) = i + j/100.0;
         
forall (i,j) in D2 do
  A2(i,j) = i + j/100.0;

writeln("A is: ", A);
writeln("A2 is: ", A2);

forall ((i,j), a) in (D, A) do
  a = j + i/100.0;

forall ((i,j), a) in (D2, A2) do
  a = j + i/100.0;

writeln("A is: ", A);
writeln("A2 is: ", A2);

forall (a, (i,j)) in (A, D) do
  a = i + j/100.0;

forall (a, (i,j)) in (A2, D2) do
  a = i + j/100.0;

writeln("A is: ", A);
writeln("A2 is: ", A2);

for i in D do
  A(i) = 1;

for i in D2 do
  A2(i) = 1;

writeln("A is: ", A);
writeln("A2 is: ", A2);
