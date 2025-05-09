# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu6/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu7/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu8/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu9/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu10/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu11/cpufreq/scaling_max_freq
# echo 1800000 | sudo tee /sys/devices/system/cpu/cpu12/cpufreq/scaling_max_freq

# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu6/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu7/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu8/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu9/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu10/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu11/cpufreq/scaling_max_freq
# echo 5300000 | sudo tee /sys/devices/system/cpu/cpu12/cpufreq/scaling_max_freq

# echo 4200000 | sudo tee /sys/devices/system/cpu/cpu17/cpufreq/scaling_max_freq
# echo 4200000 | sudo tee /sys/devices/system/cpu/cpu18/cpufreq/scaling_max_freq

for i in $(seq 0 25); do
  echo 4200000 | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
done
for i in $(seq 26 27); do
  echo 1200000 | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
done

cat /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq
cat /sys/devices/system/cpu/cpu16/cpufreq/scaling_max_freq
# echo 17
# cat /sys/devices/system/cpu/cpu17/cpufreq/scaling_max_freq
# cat /sys/devices/system/cpu/cpu18/cpufreq/scaling_max_freq

# CORES 0-15 run at 5,300,000, 16-27 at 4,200,000