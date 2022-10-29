from pacsltk import perfmodel

def predictor(arrival_rate, warm_service_time, cold_service_time, idle_time_before_kill):
        props1, _ = perfmodel.get_sls_warm_count_dist(arrival_rate, warm_service_time, cold_service_time, idle_time_before_kill)
        return props1["avg_resp_time"]

if __name__ == "__main__":
	arrival_rate = 100
	warm_service_time = 2
	cold_service_time = 3
	idle_time_before_kill = 100
	props1, _ = perfmodel.get_sls_warm_count_dist(arrival_rate, warm_service_time, cold_service_time, idle_time_before_kill)
	print(props1["avg_resp_time"])
