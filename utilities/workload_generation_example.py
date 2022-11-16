from datetime import datetime
import matplotlib.pyplot as plt
import copy
import numpy as np
import json

from dl_utilities import dl_cumsum
from bimodal import BimodalFun


# Example parameters
simple = True
average = True

# External parameters
times_seed = 500000
max_n_variations = 600 # samples
min_workload = 0.1
max_workload = 1.8
total_episode_length = 600
avg_interval = round(total_episode_length / max_n_variations)

np.random.seed(times_seed)
events1 = np.random.exponential(avg_interval, max_n_variations)

# Sum cumulatively to get the event times (and not their distances)
cumulative_events1 = dl_cumsum(events1)

# Parameters for the bimodal curve (TO BE MODIFIED / COMPLETED)
bimodal_params_1 = {
  "vmin": min_workload,
  "vmax": max_workload,
  "tmin": 0,
  "tmax": total_episode_length,
  #"a": 0.0005,
  #"b": 0.0005,
  "alpha": 1.2,
  #"seed": datetime.now().timestamp,
  "seed": 1234567,
  "peaks": []
}
bimodal_params_1["a"] = 3.33/(bimodal_params_1["tmax"] - bimodal_params_1["tmin"])**2
bimodal_params_1["b"] = 4/(bimodal_params_1["tmax"] - bimodal_params_1["tmin"])**2

cumulative_events1 = np.append([bimodal_params_1["tmin"]], cumulative_events1)
cumulative_events1 = np.append(cumulative_events1, total_episode_length)

# Generate a bimodal curve
curve_1 = BimodalFun(bimodal_params_1)
x = np.linspace(bimodal_params_1["tmin"], bimodal_params_1["tmax"],
  max(1000, min(total_episode_length*5, int(1e+4))))

# Extract the lambda values from the bimodal curve
if simple:
  w_1_1 = curve_1.eval(cumulative_events1)
if average:
  w_1_2 = curve_1.eval(cumulative_events1, True)

# Graph 1
g_1 = plt.figure()
ax = plt.gca()
ax.set_xlim([bimodal_params_1["tmin"], bimodal_params_1["tmax"]])
plt.title("Peaks and times random I")
plt.xlabel("Time")
plt.ylabel("Lambda")
plt.plot(x, curve_1.eval(x), 'b', label = "Original curve")
if simple:
  plt.hlines(w_1_1[1:], cumulative_events1[:-1], cumulative_events1[1:], 'r',
    label = "Simple sampling")
if average:
  plt.hlines(w_1_2[1:], cumulative_events1[:-1], cumulative_events1[1:], 'g',
    label = "Average sampling")
plt.legend()
#plt.show(block = False)
plt.savefig("g_1.png")

bimodal_params_2 = copy.deepcopy(bimodal_params_1)
bimodal_params_2["peaks"] = [0.05* bimodal_params_2["tmax"], 0.95* bimodal_params_2["tmax"]]

events2 = [avg_interval] * max_n_variations
cumulative_events2 = dl_cumsum(events2)
cumulative_events2 = np.append([bimodal_params_2["tmin"]], cumulative_events2)
#cumulative_events2 = np.append(cumulative_events2, total_episode_length)


# Generate a bimodal curve
curve_2 = BimodalFun(bimodal_params_2)

# Extract the lambda values from the bimodal curve
if simple:
  w_2_1 = curve_2.eval(cumulative_events2)
if average:
  w_2_2 = curve_2.eval(cumulative_events2, True)

# Graph
g_2 = plt.figure()
ax = plt.gca()
ax.set_xlim([bimodal_params_2["tmin"], bimodal_params_2["tmax"]])
plt.title("Peaks and times deterministic")
plt.xlabel("Time")
plt.ylabel("Lambda")
plt.plot(x, curve_2.eval(x), 'b', label = "Original curve")
if simple:
  plt.hlines(w_2_1[1:], cumulative_events2[:-1], cumulative_events2[1:], 'r',
    label = "Simple sampling")
if average:
  plt.hlines(w_2_2[1:], cumulative_events2[:-1], cumulative_events2[1:], 'g',
    label = "Average sampling")
plt.legend()
#plt.show(block = False)
plt.savefig("g_2.png")



np.random.seed(67676767)
events3 = np.random.exponential(avg_interval, max_n_variations)

bimodal_params_3 = copy.deepcopy(bimodal_params_1)
bimodal_params_3["peaks"] = [0.2* bimodal_params_2["tmax"], 0.8* bimodal_params_2["tmax"]]
bimodal_params_3["seed"] = 20202020
bimodal_params_3["a"] = 50/(bimodal_params_3["tmax"] - bimodal_params_3["tmin"])**2
bimodal_params_3["b"] = 50/(bimodal_params_3["tmax"] - bimodal_params_3["tmin"])**2


# Sum cumulatively to get the event times (and not their distances)
cumulative_events3 = dl_cumsum(events3)
cumulative_events3 = np.append([bimodal_params_3["tmin"]], cumulative_events3)
cumulative_events3 = np.append(cumulative_events3, total_episode_length)

# Generate a bimodal curve
curve_3 = BimodalFun(bimodal_params_3)

# Extract the lambda values from the bimodal curve
if simple:
  w_3_1 = curve_3.eval(cumulative_events3)
if average:
  w_3_2 = curve_3.eval(cumulative_events3, True)

# Graph
g_3 = plt.figure()
ax = plt.gca()
ax.set_xlim([bimodal_params_3["tmin"], bimodal_params_3["tmax"]])
plt.title("Peaks and times random II")
plt.xlabel("Time")
plt.ylabel("Lambda")
plt.plot(x, curve_3.eval(x), 'b', label = "Original curve")
if simple:
  plt.hlines(w_3_1[1:], cumulative_events3[:-1], cumulative_events3[1:], 'r',
    label = "Simple sampling")
# if average:
#   plt.hlines(w_3_2[1:], cumulative_events3[:-1], cumulative_events3[1:], 'g',
#     label = "Average sampling")
plt.legend()
#plt.show(block = False)
plt.savefig("g_3.png")

# get maximum workload (.95 percentile)
max_lam = np.percentile(w_3_1, 95)

#1 hour scenario: reconfiguration each 5 min => 12 total reconfig
total_reconfig1 = 12
win_len1 = np.int32(max_n_variations / total_reconfig1)
temp1 = np.reshape(w_3_1[1:-1], (-1, win_len1))
means1 = np.mean(temp1, axis=1)
means1 = np.insert(means1, 0, max_lam)
fig1, ax1 = plt.subplots(figsize=(8, 6), layout = 'constrained')
x1 = np.arange(0, 61, 5)
ax1.set_xticks(x1)
ax1.set_xlabel("Time [min]")
ax1.set_ylabel("Worload [requests/sec]")
ax1.set_title("Lambda profile 1 hour scenario")
ax1.plot(x1[1:], means1[1:], 'bx-')
ax1.plot(x1[0], means1[0], 'ro')
plt.savefig("hour1.png")


#2 hour scenario: reconfiguration each 5 min => 24 total reconfig
total_reconfig2 = 24
win_len2 = np.int32(max_n_variations / total_reconfig2)
temp2 = np.reshape(w_3_1[1:-1], (-1, win_len2))
means2 = np.mean(temp2, axis=1)
means2 = np.insert(means2, 0, max_lam)
fig2, ax2 = plt.subplots(figsize=(16, 6), layout = 'constrained')
x2 = np.arange(0, 121, 5)
ax2.set_xticks(x2)
ax2.set_xlabel("Time [min]")
ax2.set_ylabel("Worload [requests/sec]")
ax2.set_title("Lambda profile 2 hours scenario")
ax2.plot(x2[1:], means2[1:], 'bx-')
ax2.plot(x2[0], means2[0], 'ro')
plt.savefig("hour2.png")


# save numpy array yo json
json_str = json.dumps({"lambda": w_3_1.tolist()})
with open("lambda.json", 'w') as f:
    f.write(json_str)


# np.random.seed(886655)
# events4 = events2
#
# bimodal_params_4 = copy.deepcopy(bimodal_params_1)
# bimodal_params_4["seed"] = 98
# bimodal_params_4["a"] = 10/(bimodal_params_4["tmax"] - bimodal_params_4["tmin"])**2
# bimodal_params_4["b"] = 70/(bimodal_params_4["tmax"] - bimodal_params_4["tmin"])**2
# bimodal_params_4["alpha"] = 4
#
#
# # Sum cumulatively to get the event times (and not their distances)
# cumulative_events4 = dl_cumsum(events4)
# cumulative_events4 = np.append([bimodal_params_4["tmin"]], cumulative_events4)
# cumulative_events4 = np.append(cumulative_events4, total_episode_length)
#
# # Generate a bimodal curve
# curve_4 = BimodalFun(bimodal_params_4)
#
# # Extract the lambda values from the bimodal curve
# if simple:
#   w_4_1 = curve_4.eval(cumulative_events4)
# if average:
#   w_4_2 = curve_4.eval(cumulative_events4, True)
#
# # Graph
# g_4 = plt.figure()
# ax = plt.gca()
# ax.set_xlim([bimodal_params_4["tmin"], bimodal_params_4["tmax"]])
# plt.title("Peaks random, times deterministic")
# plt.xlabel("Time")
# plt.ylabel("Lambda")
# plt.plot(x, curve_4.eval(x), 'b', label = "Original curve")
# if simple:
#   plt.hlines(w_4_1[1:], cumulative_events4[:-1], cumulative_events4[1:], 'r',
#     label = "Simple sampling")
# if average:
#   plt.hlines(w_4_2[1:], cumulative_events4[:-1], cumulative_events4[1:], 'g',
#     label = "Average sampling")
# plt.legend()
# #plt.show(block = False)
# plt.savefig("g_4.png")
#
# # plt.pause(0.001) # Pause for interval seconds.
# # input("Hit [enter] to end.")
# # plt.close('all')
