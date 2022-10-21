# Weisfeiler-Leman Coloring Algorithm ###

Implements the Weisfeiler-Leman coloring algorithm on directed graphs with edge/vertex colors.

Built from the code for the Amenability Test by Christopher Morris (https://github.com/chrsmrrs/amenability_test.git).

We currently only support graphs specified in ASP (clingo) programs.
Future work includes supporting more graph formats.

We provide two implementations, one in C++ and another in python. Both are basically equal: the accept the same
options and arguments. The executable is called with the filename containing the directed colored graph. Options
are:

* ```--disable-chosen-labels``` for not using the atoms ```choose()``` in the file that specifies which labels
to take into account. With this option, all labels are considered.
* ```--normalize-colors``` vertex colors are normalized after each iteration. This reduces the chances of overflows,
but it is common to have them.
* ```--uniform-initial-coloring``` use an initial uniform coloring for nodes.

The folder ```examples/``` contains some example graphs. An example execution is the following:
```
$ ./src/wl --disable-chosen-labels --normalize-colors examples/blocks1_3.lp 
Reading file 'examples/blocks1_3.lp' ...
graph: #nodes=22, #edges=42, #edge-labels=4
WL: #iterations=2, #colors=5, elapsed-time=0.000107166
6 node(s) with color 5 : 16 17 18 19 20 21
6 node(s) with color 4 : 10 11 12 13 14 15
6 node(s) with color 3 : 4 5 6 7 8 9
3 node(s) with color 2 : 1 2 3
1 node(s) with color 1 : 0
total 22 node(s)
color(21,5)
color(20,5)
color(19,5)
color(18,5)
color(17,5)
color(16,5)
color(15,4)
color(14,4)
color(13,4)
color(12,4)
color(11,4)
color(10,4)
color(9,3)
color(8,3)
color(7,3)
color(6,3)
color(5,3)
color(4,3)
color(3,2)
color(2,2)
color(1,2)
color(0,1)
```

The output tells the final color of each vertex. The python call is as follows:

```
$ python python/wl.py --disable-chosen-labels --normalize-colors examples/blocks1_3.lp 
graph: #nodes=22, #edges=42, #edge-labels=4
/Users/blaibonet/software/github/weisfeiler-leman/python/color_refinement/color_refinement.py:138: RuntimeWarning: overflow encountered in long_scalars
  r = a * a + a + b if a >= b else a + b * b
WL: #iterations=2, #colors=5, elapsed_time=0.0007749169999999861
1 node(s) with color '1': 0
3 node(s) with color '2': 1 2 3
6 node(s) with color '3': 4 5 6 7 8 9
6 node(s) with color '4': 10 11 12 13 14 15
6 node(s) with color '5': 16 17 18 19 20 21
total 22 node(s)
color(0,1)
color(1,2)
color(2,2)
color(3,2)
color(4,3)
color(5,3)
color(6,3)
color(7,3)
color(8,3)
color(9,3)
color(10,4)
color(11,4)
color(12,4)
color(13,4)
color(14,4)
color(15,4)
color(16,5)
color(17,5)
color(18,5)
color(19,5)
color(20,5)
color(21,5)
```

The output is the same. As it can be seen, there are overflows when computing the "pairing" function (the warning is emitted only for the first overflow). This is due that 64-bits integers are used rather than python bignums which are of arbitrary precision but much slower.
