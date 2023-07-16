ESPChat  
!["demopic 1"](images/demopic1.PNG?raw=true )
 - примитивный чатик на контроллере ESP32 (wifi-вебсервер c хранением сообщений на флешке контроллера)
 - реализован как captive portal - открытие при подключении к wifi от контроллера

Зачем это? 
 - пока не понятно, больше учебный проект. Но можно пофантазировать..
 
Как запустить?
 - приобрести контроллер esp32 (dev kit)
 - установить среду Arduino IDE https://www.arduino.cc/en/software
 - в среде Arduino IDE установить библиотеки ESP32 по инструкции https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
 - установить плагин esp32fs - загрузчик файлов  в файловую систему контроллера по инструкции https://github.com/lorol/arduino-esp32fs-plugin Installation
 - отредактировать скетч, выставить SSID и пароль точки WIFI, которая будет вещаться ESP32, если закомментировать пароль - точка доступа будет публичной
 - выбрать свою модель контроллера, порт и загрузить в него скетч
 - в меню Интрументы-> Esp32 Sketch data upload - загрузить файлы вебсайта на флешку контроллера (автоматически берутся из каталога data), если пункт отсутствует - неверно установлен esp32fs
 - пробуем подключаться к точке доступа, должен открыться чатик, но как - зависит от девайса на котором будет открываться, если не появляется, то открыть вручную http://192.168.1.4/
 

проблемы:
 - медленное чтение сообщений с флешки и соответственно загрузка в чат, видимо нужно допилить кэширование
 - сложности при загрузке на esp32 s2 mini (порты перескакивают)

 
