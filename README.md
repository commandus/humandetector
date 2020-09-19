# human-detector

Опрашиваает COM port, подключенный через USB Digispark Attiny85.

Рабтает с заданной задержкой (в 1 или 2 секунды). 

В течение временого окна выделяет максимальную температуру, и, если
температура находится в заданном диапазоне, выдает ее в stdout.

[Репозиторий irthermometer](https://gitlab.com/commandus/irthermometer.git)

[Репозиторий human-detector](https://gitlab.com/commandus/humandetector.git)

## Схема передачи данных с датчика

```
    +---------------------+  32-42C  +--------+ 
    |    human-detector   |--------->| Max T  |
    +---------------------+    2s    +--------+
           | 
           | USB serial port
           |
    +--------------+
    |   Digispark  |
    |   AtTiny85   |
    +--------------+
           |
           |                           ИК термометр
    +---------------+               +------------------+
    |   I2C bus     | <-------------| Melexis MLX90614 | 
    +---------------+               +------------------+
```

## Зависимости

- libev

```
sudo apt install libev-dev
```

### Linux

```
./autogen.sh 
./configure
make
sudo make install
```

### Windows

```
mkdir build
cd build
cmake ..
make
cpack
```
