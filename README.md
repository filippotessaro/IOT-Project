# Comparing Neighbor Discovery Protocols

To run the simulation, it is needed to set the right primitive by uncommenting the right one in the app.c

Then to run a batch of five simulations the following bash command is needed:

```
# for BURST
python3 run-all.py

# for SCATTER 
python3 run-all-scatter.py
```

All the two .py scripts will automate the process of compile, do five cooja simulation and prepare the .csv and all the files useful for the analysis.

In the end, to get the plots, execute the script utils/neigh-discover.R in the following way:

```
script neigh-discover.R -i ../dst_test/scatter_15task -m "scatter"
# where -i is the input dir without _sim in the name
# and -m is the type of primitive 
```

Plots will automatically be stored in dst_test/out/ directory.

