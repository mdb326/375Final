# rm test
rm bank
# g++ -o test test.cpp -lpthread -O3
g++ -std=c++17 -o bank bank3.cpp -pthread -O3
# ./test
sudo ./bank