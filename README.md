# human-detector

Опрашивает USB COM port Digispark Attiny85.

Рабтает с заданной задержкой, равной временному окну. 

В течение временного окна программа выделяет максимальную температуру, и, если
температура находится в заданном диапазоне, выдает ее в stdout в градусах Кельвина, снабжая ее временной меткой.

[Репозиторий irthermometer](https://gitlab.com/commandus/irthermometer.git)

[Репозиторий human-detector](https://gitlab.com/commandus/humandetector.git)

## Схема передачи данных с датчика

Контроллер USB Digispark Attiny85 имеет загрузчик, который при подключении к USB порту в
течение 5 секунд ожидает подключения программатора, устанавливаемого в Arduino IDE.

По истечении 5 секунд он передает управление загруженному в контроллер коду. При этом меняется описание устройства.

Поэтому при подключении к USB порту нужно ждать 5 секунд.

В Windows нужно устанавливать драйвер. В Linux, в часности, в Ubuntu, нужно прописывать правила USB.

Программа может послать один из трех символов в COM порт (в скобках значение опции -m, --mode):

- '0' запрос температуры ИК датчика (ir)
- '1' запрос температуры окружающей среды (корпуса датчика) (ambient)
- '2' запрос числа тиков. Используется для отладки (tick)
- ничего не отправлять (file)

При любом запросе включаестя или выключается второй светодиод. Первый светодиод постоянно горит при полдключении к USB порту.

В ответ по COM порту приходит температура (или тики).

Температура в Кельвинах, значение умноженo на 100.

У меня где-то в KDE иногда начинает возникать ошибка при подключении USB Digispark Attiny85, при этом устройство ошибочно определяется как ISDN модем.

Пример кода Digispark, как отправлять данные по COM порту, часто приводит к зависанию где то в KDE- вероятно, нужно разобраться с правильностью определения USB устройства.

```
    +---------------------+  32-42C  +--------+ 
    |    human-detector   |--------->| Max T  |
    +---------------------+    1s    +--------+
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

## Принцип работы

Задаются границы минимальной и максимвльной температуры тела. За этими пределами считается, что данных нет.

Задается временной промежуток времени, по умолчанию равный 1 секунде, в течение которого делаются измерения температуры тела.

Началом измерения считается время, когда значение температуры стало находиться в заданном диапазоне температур.

Окончанием измерения считается время, когда значение температуры вышло за пределы диапазона.

Определяется максимальное полученное значение температуры и передается в stdout с меткой времени начала измерения.

Метка времени- число секунд с 1970 года (Unix epoch).

Если измерения продолжаются больше секунды, каждую секунду (или через другой заданный промежуток времени) отправляется новое значение максимальной температуры с той же самой меткой времени.

За 1 секунду по COM порту контроллер способен передать не более 20-25 значений температуры.

## Опции

- путь устройства COM порта
- -l Число ожидаемой минимальной температуры тела в градусаз Цельсия. По умолчанию 32
- -h Число ожидаемой максимальной температуры тела в градусаз Цельсия. По умолчанию 42
- -s Число секунд временного окна. По умолчанию 1 секунда. В течение заданного промежутака времени определяется максимальная температура.
- -g K- выдает температуру в градусах Цельсия. C- в Кельвинах (по умолчанию). Градус Цельсия = K + 273.15
- -y Число миллисекунд задержки между чтениями COM порта. По умолчанию нет.


```
./human-detector -?
Usage: human-detector
 [-v?] [COM port path] [-l <number>] [-h <number>] [-s <number>] [-g K or C] [-y <ms>]
Temperature reader
  COM port path             Default /dev/ttyACM0
  -l, --t0=<number>         lo temperature threshold, C. Default 32
  -h, --t1=<number>         hi temperature threshold, C. Default 42
  -s, --seconds=<number>    Default 1
  -g, --degrees=K or C      K- Kelvin (default), C- Celcius
  -m, --mode=i, a, t or f   ir- infrared sensor, ambient- internal sensor, tick- counter value, file- read from the file
  -y, --delay=<ms>          delay to read, ms. Default 0
  -1                        output max temperature only(no output every 1s when temperature growing)
  -t, --timeformat=<format> Output time format, e.g. "%F %T". Default prints seconds since Unix epoch
  -v, --verbose             Set verbosity level
  -?, --help                Show this help
```

Ошибки при записи в устройство игнорируются.

Опция -y, --delay=<ms> задает задержку в миллисекундах перед чтением значения из устройства или файла, и полезга при отладке с чтением данных из тестовых файлов.

Опция -1 заставляет выводить только одну строку с максимальной измеренной температурой. Поэтому, пока объект не перестает видиться термометром, данные не будут переданы. 
Без этой опции, по умолчанию, строка с температурой выдается минимум через секунду (или другой заданный промежуток времени).


Опция -t, --timeformat задает формат штампа времени. Если опция не задана, выдается число секунд с 1 января 1970 года. 

Подходящие форматы:

- "%F %T%Z" с поясным временем
- "%F %T" без указания пояса
- "%FT%T%Z" тоже

Штамп времени выводит локальное время (с учетом временного пояса).

## Сборка

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

## Зависимости

argtable3 

Copyright (C) 1998-2001,2003-2011,2013 Stewart Heitmann <sheitmann@users.sourceforge.net> All rights reserved.

strptime for Windows

Copyright (c) 1999 Kungliga Tekniska Hï¿œgskolan (Royal Institute of Technology, Stockholm, Sweden). All rights reserved.


## USB idenifiers

При передаяе управления библиотеке USB Serial Port:

- Вендор 16d0
- Продукт 087e

New USB device found, idVendor=16d0, idProduct=087e, bcdDevice= 1.00
New USB device strings: Mfr=1, Product=2, SerialNumber=0
Product: Digispark Serial
Manufacturer: digistump.com

## Тесты

Тестовый файл содержит строки десятичных чисел- значений температуры в градусах Кельвина, умноженных на 100 и выглядит так:

```
09815
29815
29815
29815
...
```

Для тестовых файлов нужно 

- указать путь к файлу (безымянная опция)
- задать задержку (опция -y) в миллисекундах

```
./human-detector -m file -y 100 -t "%F %T" t1.txt
```

Опция -t "%F %T"  задает формат строки времени в текущем часовом поясе, например, 2020-10-04 14:02:02.
