from pacsltk import perfmodel

def predictor(arrival_rate, warm_service_time, cold_service_time, idle_time_before_kill):
        props1, _ = perfmodel.get_sls_warm_count_dist(arrival_rate, warm_service_time, cold_service_time, idle_time_before_kill)
        return props1["avg_resp_time"]
