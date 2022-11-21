from datetime import datetime
import matplotlib.pyplot as plt
import copy
import numpy as np
import json

plt.rcParams['font.size'] = '16'

def fun(file_names, lambdas_file, time_scenario):
    data = dict()
    for scenario, file_name in file_names.items():
        f = open(file_name)
        data[scenario] = json.load(f)
    lambdas = dict()
    with open(lambdas_file, 'r') as f:
        lambdas = json.load(f)
    if time_scenario == 1: # 1 hour scenario
        x = np.arange(0, 61, 5)
        #plot costs
        fig_cost, ax_cost = plt.subplots(figsize=(12, 8))
        ax_cost.set_xticks(x)
        for scenario, _ in file_names.items():
            ax_cost.plot(x, data[scenario]["Costs"], 'o-', label = scenario)
        ax_cost.set_xlabel("time [minutes]")
        ax_cost.set_ylabel("Cost [$/hour]")
        ax_cost.legend()
        plt.savefig("Costs.png", bbox_inches='tight')
        #plot num resources
        resources_type = ["Edge", "VM", "Faas"]
        for type in resources_type:
            fig_num, ax_num = plt.subplots(figsize=(12, 8))
            ax_num.set_xticks(x)
            for scenario, _ in file_names.items():
                ax_num.plot(x, np.array(data[scenario]["NumberActiveResources"][type]), 'o-', label = scenario)
            ax_num.set_xlabel("time [minutes]")
            ax_num.set_ylabel("Number")
            ax_num.legend()
            ax_num.set_title("{} active devices".format(type))
            plt.savefig("{}_num.png".format(type), bbox_inches='tight')

if __name__ == '__main__':
    file_names = {
        "Scenario1" : "Sol_RunningExample_system_description_scenario1.json",
        "Scenario2" : "Sol_RunningExample_system_description_scenario2.json",
        "Scenario3" : "Sol_RunningExample_system_description_scenario3.json"}
    lambdas_file = "lambda_1hour.json"
    time_scenario = 1
    fun(file_names, lambdas_file, time_scenario)
