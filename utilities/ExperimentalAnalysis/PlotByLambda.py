from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
import copy
import numpy as np
import json

plt.rcParams['font.size'] = '18'

def fun(file_names):
    data = dict()
    for scenario, file_name in file_names.items():
        f = open(file_name)
        data[scenario] = json.load(f)
    # lambdas = dict()
    # with open(lambdas_file, 'r') as f:
    #     lambdas = json.load(f)
    x = np.arange(0, 121, 5)
    labels = [m if m in np.arange(10,121,10) else "" for m in np.arange(5,121,5)]
    labels = [0] + labels
    fontsize='small'
    #plot costs
    for scenario, _ in file_names.items():
        fig_cost, ax_cost = plt.subplots(figsize=(12, 8))
        ax_cost.set_xticks(x)
        ax_cost.set_xticklabels(labels)
        ax_cost.plot(x[0:], data[scenario]["Costs"][0:], 'o-', markersize=3)
        ax_cost.plot(x[0], data[scenario]["Costs"][0], 'ro', markersize=3)
        ax_cost.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
        ax_cost.set_xlabel("time [minutes]")
        ax_cost.set_ylabel("Cost [$/hour]")
        plt.savefig("Costs_{}.png".format(scenario), bbox_inches='tight')

        # resources_type = ["Edge", "VM", "FaaS"]
        # #plot num resources
        # fig_num, ax_num = plt.subplots(figsize=(12, 8))
        # ax_num.set_xticks(x)
        # ax_num.set_xticklabels(labels)
        # max_num = 0;
        # for r_type in resources_type:
        #     ax_num.plot(x[1:], data[scenario]["NumberActiveResources"][r_type][1:], 'o-', label = r_type, markersize=3)
        #     if max(data[scenario]["NumberActiveResources"][r_type]) > max_num:
        #         max_num = max(data[scenario]["NumberActiveResources"][r_type])
        # ax_num.set_yticks(np.arange(0, max_num+1, 1, dtype=int))
        # ax_num.set_xlabel("time [minutes]")
        # ax_num.set_ylabel("Number of Active Nodes")
        # ax_num.legend(fontsize=fontsize)
        # #ax_num.set_title("{} active devices".format(type))
        # plt.savefig("NumberResources_{}.png".format(scenario), bbox_inches='tight')
        # #plot cost ratio
        # fig_cost_rat, ax_cost_rat = plt.subplots(figsize=(12, 8))
        # ax_cost_rat.set_xticks(x)
        # ax_cost_rat.set_xticklabels(labels)
        # for r_type in resources_type:
        #     ax_cost_rat.plot(x[1:], np.array(data[scenario]["FractionCostByRes"][r_type][1:]), 'o-', label = r_type, markersize=3)
        # ax_cost_rat.set_xlabel("time [minutes]")
        # ax_cost_rat.set_ylabel("Cost Fraction")
        # ax_cost_rat.legend(fontsize=fontsize)
        # plt.savefig("CostRatio_{}.png".format(scenario), bbox_inches='tight')
        #
        # #plot component perf
        # fig_perf, ax_perf = plt.subplots(figsize=(12, 8))
        # ax_perf.set_xticks(x)
        # ax_perf.set_xticklabels(labels)
        # c = 0
        # for path, value in data[scenario]["GlobalConstraints"].items():
        #     ax_perf.plot(x[1:], value["response_time"][1:], color="C{}".format(c), marker = 'o', linestyle = '-', label = path.upper(), markersize=3)
        #     ax_perf.hlines(value["threshold"], xmin = x[0], xmax = x[-1], linestyle = 'dotted', linewidth = 2, color="C{}".format(c))
        #     c = c + 1
        # ax_perf.set_xlabel("time [minutes]")
        # ax_perf.set_ylabel("Response time [seconds]")
        # ax_perf.legend(fontsize=fontsize)
        # plt.savefig("Response_times_{}.png".format(scenario), bbox_inches='tight')
        #
        # print("{} Execution times".format(scenario))
        # print("Mean: ", np.mean(data[scenario]["Timings"]))
        # print("Standard deviation ", np.std(data[scenario]["Timings"]))
        # print("\n")

    #plot cost saving
    cs = lambda c, c0 : (1-np.array(c)/c0) * 100
    fig_s, ax_s = plt.subplots(figsize=(12, 8))
    ax_s.set_xticks(x)
    ax_s.set_xticklabels(labels)
    for scenario, _ in file_names.items():
        if scenario == "Scenario1":
            label = "Scenario A"
        elif scenario == "Scenario2":
            label = "Scenario B"
        else:
            label = "Scenario C"

        print("{} Mean Cost Saving".format(scenario))
        print("Mean: ", np.mean(cs(data[scenario]["Costs"], data[scenario]["Costs"][0])))
        print("Standard deviation ", np.std(cs(data[scenario]["Costs"], data[scenario]["Costs"][0])))
        print("\n")

        ax_s.plot(x[1:], cs(data[scenario]["Costs"], data[scenario]["Costs"][0])[1:], 'o-', markersize=3, label = label)

    ax_s.legend(fontsize=fontsize)
    ax_s.yaxis.set_major_formatter(FormatStrFormatter('%d'))
    ax_s.set_xlabel("time [minutes]")
    ax_s.set_ylabel("Cost Saving [%]")
    plt.savefig("CostSavingUseCase.png", bbox_inches='tight')



if __name__ == '__main__':
    # file_names = {
    #     "Strict_5Components" : "Strict/Sol_5Components.json",
    #     "Strict_10Components": "Strict/Sol_10Components.json",
    #     "Strict_15Components": "Strict/Sol_15Components.json",
    #     "Light_5Components"  : "Light/Sol_5Components.json",
    #     "Light_10Components" : "Light/Sol_10Components.json",
    #     "Light_15Components" : "Light/Sol_15Components.json"
    #     }
    file_names = {
    # "Scenario1" : "UseCase/Sol_RunningExample_system_description_scenario1.json",
    # "Scenario2" : "UseCase/Sol_RunningExample_system_description_scenario2.json",
    "Scenario3" : "UseCase/Sol_Scenario3.json"
    }
    #lambdas_file = "lambda_1hour.json"
    fun(file_names)
