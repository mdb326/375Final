# rm test
rm bank
# g++ -o test test.cpp -lpthread -O3
sh cores.sh
g++ -std=c++17 -o bank bank.cpp -pthread -O3
# ./test
sudo ./bank