How to compile benchmark:
  g++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -lboost_filesystem

  With PGO:
  first: g++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -DNDEBUG -lboost_filesystem -fprofile-generate
  second: g++ ./benchmark.cpp -Ofast -o benchmark -march=native -flto=auto -DNDEBUG -lboost_filesystem -fprofile-use -fprofile-correction

Compile ui.cpp:
  g++ ./ui.cpp -Ofast -o constellation.out -march=native -flto=auto $(pkg-config --cflags --libs gtkmm-4.0)