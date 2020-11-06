# Электронный журнал термометрии

Электронный журнал термометрии предназначен для упрощения ведения журнала термомтетрии сотрудников.

## Требуемое обороудование

- ИК датчик температуры Melexis, подключенный к шине I2C
- Digispark Attiny85, подключенный к шине I2C с USB разъемом
- Компьютер под управлением GBU Linux с USB разъемом или планшет Android ([android-irthermometer](git@gitlab.com:commandus/android-irthermometer.git))
- USB RFID сканер (операционной системой компьютера определяется как USB клавиатура)
- Турникет со СКУД Sigur, данные о проходе сотруднка по RFID карте берутся из базы данных Sigur. Для этого компьютер должен быть подключен 
к локальной вычислительной сети кабелем или беспроводно (Wi-Fi).

Примечание: так как USB RFID сканер работает как клавиатура, считанный ею скан-код передается операционной системой или оконным менеджеором в устройство вывода, находящееся в фокусе пользователя.

## Состав программного обеспечения

Электронный журнал термометрии представляет собой набор программ, работающих под управлением ОС GNU Linux
и запускаемых из shell скрипта.

Репозитории кода

- [Arduino](git@gitlab.com:commandus/irthermometer.git) - устанавливается в USB stick
- [Android](git@gitlab.com:commandus/android-irthermometer.git) - программа для показа температуры на планшете и отправки в веб сервис
- [GNU Linux](git@gitlab.com:commandus/human-detector.git) - программа для передачи температуры в базу данных и/или веб-сервис

Далее рассматривается приложение для ОС GNU Linux.

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

Метка времени- число миллисекунд с 1970 года (Unix epoch).

Если измерения продолжаются больше секунды, каждую секунду (или через другой заданный промежуток времени) отправляется новое значение максимальной температуры с той же самой меткой времени.

За 1 секунду по COM порту контроллер способен передать не более 20-25 значений температуры.


## Использование программ в оболочке и скриптах

Каждая программа может быть запущена в двух режимах:
- все параметры передаются через опции (нужно указать опцию -R)
- обработка строк из stdin (по умолчанию) - пераметры, не заданные опциями, передаются по конвейеру

При первом способе программам нужно все параметры передавать через опции программ. 
Это более гибкий способ, но, вероятно, избыточный.

При втором способе программа читает строку и копирует ее в stdout, и может
добавить свои данные в конец этой строки. Строка содержит опции. Второй способ покороче.

Это используется в конвейере команд shell'а для передачи параметров по цепочке.

Например, в таком конвейере:

```
./human-detector /dev/ttyACM1 | ./sigur-last-card | ./put-temperature-db | ./put-temperature-json | jq .
```

- human-detector читает температуру с Digispark Attiny85 (устройство /dev/ttyACM1) и передает ее в конвейер как параметры --temperature <Температура, C> и другие
- sigur-last-card читает переданное из конвейера и добавляет параметры --card <Номер Карты Сотрудника> --timein <Врема Прохода Через Турникет>
- put-temperature-db получает температуру, номер карты и записывает в базу данных, затем добавляет  параметр --id <номер записи в БД> 
- put-temperature-json получает температуру, номер карты и отправляет их в веб сервис 

Символ | конвейера в shell'е создает канал (pipe) между stdout первой программы и stdin второй. Поэтому stdin и stdout (с номерами файлов 0 и 1)
заняты каналом для передачи параметров.

Для использования программы usb-rfid-card, которая читает номер RFID карты как устройство клавиатуры, нужно сделать небольшой трюк:

- при запуске переименовать связанный с USB RFID сканером (клавиатурой) stdin номер файла 0 в другой, например, 6
- передать файл с этим номером 6 в usb-rfid-card через отдельную опцию -k

Вот скрипт send-temp-svcЮ который считывает данные с USB RFID сканера:

```
#!/bin/sh
exec 6<&0
./human-detector /dev/ttyACM1 | ./usb-rfid-card -k 6 | ./put-temperature-db | ./put-temperature-json | jq .
exec 0<&6 6<&-
exit 0
```

По окончании pipe файл 6 переименовывается в файл 0 обратно.

## Программы

- human-detector опрашивает термометр через USB
- usb-rfid-card получает номер карты сотрудника от сканера RFID меток через USB (USB клавиатура)
- sigur-last-card получает номер карты сотрудника из базы данных Sigur, прошедшего через турникет
- put-temperature-db записывает данные в базу данных
- put-temperature-json передает данные в веб-сервис

Программы передаеют следующие значения:


|  Название            | Параметр                                         | 
| -------------------- | ------------------------------------------------ |
| gate                 | Номер турникета                                  |
| card                 | Номер карты сотрудника                           |
| time                 | Время начаола измерения в msс 1970 г.            |
| t                    | Температура с учетом коэффициента излучения 0.92 |
| tir                  | Температура, измеренная датчиком                 |
| tmin                 | Температура минимальная с датчика (окружения)    |
| tambient             | Температура корпуса датчика (всегда 0)           |
| id                   | Номер записи в базе данных                       |
| timein               | Время прохода через турникет Sigur, сек с 1970 г.|

* Темература в градусах Цельсия, умноженная на 100


|  Программа           | Ожидаемый параметр | Передаваемый параметр |
| -------------------- | ------------------ | --------------------- |
| human-detector       |                    | time                  |
|                      |                    | t                     |
|                      |                    | tir                   |
|                      |                    | tmin                  |
|                      |                    | tambient              |
|                      |                    |                       |
| usb-rfid-card        |                    |                       |
|                      |                    | card                  |
| sigur-last-card      |                    |                       |
|                      |                    | card                  |
|                      |                    | timein                |
| put-temperature-db,  |                    |                       |
| put-temperature-json | gate               |                       |
|                      | card               |                       |
|                      | time               |                       |
|                      | t                  |                       |
|                      | tir                |                       |
|                      | tmin               |                       |
|                      | tambient           |                       |
|                      |                    |                       |

### human-detector

Опрашивает USB COM port Digispark Attiny85.

Рабтает с заданной задержкой, равной временному окну. 

В течение временного окна программа выделяет максимальную температуру, и, если
температура находится в заданном диапазоне, выдает ее в stdout в градусах Кельвина, снабжая ее временной меткой.

human-detector определяет максимальную температуру. Измерение делается несколько раз в течении 1 секунды.
Если измеряемый объект продолжит измерения, не отрывая руки, human-detector пошлет второе измерение следующую секунду, 
если будет достигнуто большее значение температуры, и так будет повторяться каждую секунду.

Но все эти измерения будет иметь один штамп времени.

Если же измеряемый объект оторвет руку, а затем приложит зханово, будет отправлено измрение с новым штампом времени.

Опция -1 заставляет human-detector ожидать максимума измерения неопределенное время.

Опция -s <число секунд> позволяет изменить время измерения. Если задать -s 0, human-detector быдет выдавать все подряд,
не находя максимум.

Режим симуляции задается опцией -m file. В режиме симуляции конвейером нужно подать данных из файла со значениями температуры 
в сотых долях градуса Цельсия:

```
30455
30445
30435
30425
30415
...
```

Опция -y задает задержку чтения строк из файла в миллисекундах.

В режиме симуляции устройство не читается и не пишется.

В режиме -m ambient с устройства выводится температура корпуса, -m tick - номер ошибки I2C.

### Опции

- COM port path             по умолчанию /dev/ttyACM0
-  --timeout=<seconds>       Таймаут COM порта, по умолчанимю 1 секунда
-  -R, --no-read             не для pipe режима (все передается в опциях)
-  -w, --wait                ждать CRLF
-  -l, --t0=<number>         Число ожидаемой минимальной температуры тела в градусах Цельсия. По умолчанию 32
-  -h, --t1=<number>         Число ожидаемой максимальной температуры тела в градусах Цельсия. По умолчанию 42
-  -s, --seconds=<number>    Время измерения, по умолчанию 1 секунда. В течение заданного промежутака времени определяется максимальная температура.
-  -g, --degrees=K or C      K- выдает температуру в градусах Цельсия. C- в Кельвинах (по умолчанию). Градус Цельсия = K + 273.15
-  -m, --mode=i, a, t, f     Режим ir- infrared sensor, ambient- internal sensor, tick- counter value, file- read from the file
-  -1                        Не ограничивать время измерения температуры 1 секундой
-  -r, --reconnect           re-open COM port on error
-  -t, --timeformat=<format> Формат штампа времени, напримре, "%F %T". По умолячаниюб выводит число миллисекунд с 1970 года
-  -y, --delay=<ms>          Таймаут в миллисекундах для симуляции данных из файла, по умолчанию без задержек.
-  -v, --verbose             Set verbosity level

Ошибки при записи в устройство игнорируются.

Опция -y, --delay=<ms> задает задержку в миллисекундах перед чтением значения из устройства или файла, и полезна при отладке с чтением данных из тестовых файлов.

Опция -1 заставляет выводить только одну строку с максимальной измеренной температурой. Поэтому, пока объект не перестает видиться термометром, данные не будут переданы. 

Без этой опции, по умолчанию, строка с температурой выдается минимум через секунду (или другой заданный промежуток времени).

Опция -t, --timeformat задает формат штампа времени. Если опция не задана, выдается число миллисекунд с 1 января 1970 года. 

Подходящие форматы:

- "%F %T%Z" с поясным временем
- "%F %T" без указания пояса
- "%FT%T%Z" тоже

Штамп времени выводит локальное время (с учетом временного пояса).

## Файлы конфигурации

Файлы конфигурации необязательны, но позволяеют скрыть чувствительную информацию, такую, как параметры соединения с СУБД,
включая имена полдьзователей СУБД и их пароли, которую можно задавать опциями командной строки.

Файлы конфигурации размещаются в домашнем каталоге пользователя

- ~/.put-temperature-db конфигурация сервера базы данных (строка подключения к РСУБД PostgreSQL)
- ~/.put-temperature-json адрес веб-сервиса

.put-temperature-db содержит строки:

 - conninfo
 - gate id number
 - gate secret number
 
.put-temperature-json содержит строки:

 - URL
 - gate id number
 - gate secret number
 
## Сборка

### Linux

Autoconf:

```
./autogen.sh 
./configure
make
sudo make install
```

cmake

```
mkdir build
cd build
cmake ..
make
sudo make install
```

### Windows

Не компилируется и не собирается пока

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

## sigur

Зависимости

- libmysqlclient-dev