# Experimental Analysis

Some common parameters:
- RG_MaxIter = 10000
- LS_MaxIter = 10000
- T = 2 (hours)

## Use Case
In the corresponding folder you find the plots (self explanatory) and two types of `.json` files:
- _Sol_Scenario*.json_: for each scenario, it contains some summary statistics on the solutions, used to generate the plots.
- _LambdaValues_Scenario*.json_: for each scenario, it stores the $\lambda$ values from the discretized workload profile.

Moreover, in the folder `SystemConfigurations` you find both the system descriptions and the initial deployment (saved as _DTbest_sol.json_).

## Scalability Analysis
In the corresponding folder you find the plots (self explanatory) and two sub-folders (`SolLight/` and `SolStrict/`), containing the summary statistics for each scenario (5, 10, 15 components) averaged on the 10 random DAG instances. The file `LambdaValues.json` represents the discretized workload profile. In the folder `SystemConfigurations` you find the system descriptions for both the light and strict frameworks. The initial deployments are saved as _best_sol_1.8.json_.
