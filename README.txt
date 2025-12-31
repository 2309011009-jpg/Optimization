How to compile benchmark:
  g++ ./benchmark.cpp -O3 -o benchmark -march=native -flto=auto -lboost_filesystem

  With PGO:
  first: g++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -DNDEBUG -lboost_filesystem -fprofile-generate
  second: g++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -DNDEBUG -lboost_filesystem -fprofile-useg++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -DNDEBUG -lboost_filesystem -fprofile-use