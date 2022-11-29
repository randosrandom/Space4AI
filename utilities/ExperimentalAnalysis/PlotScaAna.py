from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter
import copy
import numpy as np
import json

plt.rcParams['font.size'] = '18'

def fun(file_names):
    x = np.arange(0, 121, 5)
    labels = [m if m in np.arange(10,121,10) else "" for m in np.arange(5,121,5)]
    labels = [0] + labels
    fontsize='small'
    data_ligth = dict()
    data_strict = dict()
    for scenario, names in file_names.items():
        f1 = open(names[0])
        f2 = open(names[1])
        data_light = json.load(f1)
        data_strict = json.load(f2)

        #plot costs
        fig_cost, ax_cost = plt.subplots(figsize=(12, 8))
        ax_cost.set_xticks(x)
        ax_cost.set_xticklabels(labels)
        ax_cost.plot(x[1:], data_light["Costs"][1:], 'o-', markersize=3, label = "Light")
        ax_cost.plot(x[1:], data_strict["Costs"][1:], 'o-', markersize=3, label = "Strict")
        ax_cost.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
        ax_cost.set_xlabel("time [minutes]")
        ax_cost.set_ylabel("Cost [$/hour]")
        ax_cost.legend(fontsize=fontsize)
        plt.savefig("Costs_{}.png".format(scenario), bbox_inches='tight')

        #timings
        fig_time, ax_time = plt.subplots(figsize=(12, 8))
        ax_time.set_xticks(x)
        ax_time.set_xticklabels(labels)
        ax_time.plot(x[1:], data_light["Timings"][1:], 'o-', markersize=3, label = "Light")
        ax_time.plot(x[1:], data_strict["Timings"][1:], 'o-', markersize=3, label = "Strict")
        ax_time.yaxis.set_major_formatter(FormatStrFormatter('%.3f'))
        ax_time.set_xlabel("time [minutes]")
        ax_time.set_ylabel("Tool execution time [seconds]")
        ax_time.legend(fontsize=fontsize)
        plt.savefig("ExeTime_{}.png".format(scenario), bbox_inches='tight')

        print("{} Execution times".format(scenario))
        print("Mean Light: ", np.mean(data_light["Timings"]))
        print("Mean Strict: ", np.mean(data_strict["Timings"]))
        print("Standard deviation Light ", np.std(data_light["Timings"]))
        print("Standard deviation Strict ", np.std(data_strict["Timings"]))
        print("\n")

    #Plot cost saving
    cs = lambda c, c0 : (1-np.array(c)/c0) * 100
    fig_s, ax_s = plt.subplots(figsize=(12, 8))
    ax_s.set_xticks(x)
    ax_s.set_xticklabels(labels)
    for scenario, name in file_names.items():
        f1 = open(name[1])
        data_strict = json.load(f1)

        if scenario == "5Components":
            label = "Scenario 1"
        elif scenario == "10Components":
            label = "Scenario 2"
        else:
            label = "Scenario 3"

        c0 = max(data_strict["Costs"][1:]) #data_strict["Costs"][0]


        print("{} Mean Cost Saving".format(scenario))
        print("Mean: ", np.mean(cs(data_strict["Costs"], c0)))
        print("Standard deviation ", np.std(cs(data_strict["Costs"], c0)))
        print("\n")

        ax_s.plot(x[1:], cs(data_strict["Costs"], c0)[1:], 'o-', markersize=3, label = label)

    ax_s.legend(fontsize=fontsize)
    ax_s.yaxis.set_major_formatter(FormatStrFormatter('%d'))
    ax_s.set_xlabel("time [minutes]")
    ax_s.set_ylabel("Cost Saving [%]")
    plt.savefig("CostSavingScaAna.png", bbox_inches='tight')

if __name__ == '__main__':
    file_names = {
        "5Components" : ["Light/Sol_5Components.json", "Strict/Sol_5Components.json"],
        "10Components": ["Light/Sol_10Components.json", "Strict/Sol_10Components.json"],
        "15Components": ["Light/Sol_15Components.json", "Strict/Sol_15Components.json"]
    }
    fun(file_names)
