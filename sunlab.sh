git stash
git pull

rm test2
# rm bank
g++ -std=c++17 -o test2 test.cpp -g -pthread -O3
# sh cores.sh
# g++ -std=c++17 -o bank bank.cpp -pthread -O3
# ./test2
# sudo ./bank

git add .
git commit -m "recompile"
git push