# Parallel_Programming, 1st task

# CMake:
# Создаем директорию для сборки
mkdir build
cd build

# Генерация Makefile с использованием double (по умолчанию)
cmake ..

# Генерация Makefile с использованием float
cmake -DUSE_FLOAT=ON ..

# Сборка проекта
make

# Запуск программы
./sin_sum

# Makefile:
# Сборка с использованием double (по умолчанию)
make

# Сборка с использованием float
make TYPE=float