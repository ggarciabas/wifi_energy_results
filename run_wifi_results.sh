#!/bin/bash
file=$(echo "exec_log_"$(date +%Y-%m-%d)".txt")
date > $file
seeds="289265 285026 374678 892680 267727 259511 398959 203588 461202 975383 93767 343737 995274 982715 559338 507994 593713 527058 961394 296231 363100 909289 399077 547217 904537 850178 17054 358793 250064"
scenarios="1 2 3"
protocols="1 2 3"
for scenario in $scenarios; do
  echo $scenario >> $file
  for seed in $seeds; do
    echo $seed >> $file
    for protocol in $protocols; do
      echo $protocol >> $file
      date >> $file
      code=$(echo time ./waf --run \"wifi_energy_results --Protocol=$protocol --Scenario=$scenario --Seed=$seed\"" >> "$file)
      echo $code >> $file
      eval $code
    done
  done
done
date >> $file
