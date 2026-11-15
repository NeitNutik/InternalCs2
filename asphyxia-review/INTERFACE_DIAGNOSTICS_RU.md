# Локализация второго отказа — 2026-11-15

Новый лог подтверждает прохождение MEM::Setup и MATH::Setup. Далее два pattern
lookup завершаются ошибкой внутри I::Setup. Старые одинаковые сообщения не позволяют
выбрать два имени из SwapChain, Input, GlobalVars, PVS, GameTraceManager. Также
GetExportAddress(CreateInterface) не доказывает успешный factory(name) вызов.

Добавлена диагностика:
- имя каждого запрошенного интерфейса, успех или factory return code;
- каждая зависимость I::Setup с именем и результатом;
- модуль и полный pattern при неудачном/неоднозначном поиске.

Это диагностическое изменение, не новая сигнатура и не гарантия успешного запуска.
Отсутствующие обязательные зависимости по-прежнему останавливают инициализацию.

Можно определить отсутствующие паттерны без пересборки/инжекта. Из корня распакованного
проекта при запущенной CS2 выполнить в PowerShell (требуется Python):

```powershell
$exe = (Get-Process cs2 -ErrorAction Stop | Select-Object -First 1).Path
$gameRoot = Split-Path (Split-Path (Split-Path $exe -Parent) -Parent) -Parent
python audit\scan_pe.py --catalog audit\patterns-patched.json --modules "$gameRoot" --output matches.json
```

В matches.json нужны результаты для core/interfaces.cpp: идентификатор, модуль,
число совпадений и hash файла. Offline-проверка сопоставляет байты файла, не значения
глобальных указателей в памяти. Для null globals/factory failures нужен новый
именованный runtime log.
