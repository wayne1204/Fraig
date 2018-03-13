## What is Fraig?
Fraig(Functionally Reduced And-Inverter Graph), is a tool that can reduce redundant logic gates in a circuit

## Getting started
Before getting started, clone the whole file into your computer, and type the following words

```
$ make clean
$ make
$ ./fraig
```
## How to use fraig
Press tab will show you some available commands, or type 'help' will show you the function of every 

### AIG file?
aig(and-inverter graph) file is a format of describing a circuit, Fraig can parse a aig file into a circuit netlist.
For more infomation about aig file please look up at http://fmv.jku.at/aiger/FORMAT.aiger

## Command of fraig
there are several action of reducing redundant gates, below are their function
- Sweep: Delete unsed gates
- Optimize: Perform simple optimization
- Strash: Structural hash
- Simulate: use binary pattern to simulate the whole circuit, do not reduce gates
- Fraig: use SAT solver to solve potential equivalent gates

