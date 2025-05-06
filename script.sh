rm test
# rm bank
g++  -std=c++17 -o test test.cpp -g -pthread -O3
# sh cores.sh
# g++ -std=c++17 -o bank bank.cpp -pthread -O3
./test
# sudo ./bank