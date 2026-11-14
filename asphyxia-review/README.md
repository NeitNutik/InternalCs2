# Asphyxia review — kindmops

[Скачать проект ZIP](asphyxia-review.zip?raw=true) · [Полный отчёт](REPORT_RU.md)

Архив содержит исправленные исходники Asphyxia, зависимости, таблицы сигнатур/смещений, patch, тесты и offline PE scanner.

Это подготовленная ревизия, а не подтверждённая поддержка текущего локального патча CS2. Актуальные игровые ABI, CRC и запуск в игре не проверены; старый CreateMove по умолчанию отключён. Полная MSVC-сборка не выполнена. Подробности внутри audit/ABI_REQUIRED.md и audit/VALIDATION.md.

Проверено: 20 016 C++ assertions с ASan/UBSan, 5 Python-тестов, применение patch к оригиналу для 21 файла.

ZIP SHA-256: `8428bff3f3be9d1035aa3a39ee068f5d5d30b199f50ad8d421a8a797b26c3ec8`
