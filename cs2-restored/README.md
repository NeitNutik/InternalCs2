Полностью заменен CS2_Dumper на output/*.hpp из a2x/cs2-dumper, commit cb01076744f3554324d76b7a4830f0fcdfde95fb. Снимок build 14197 от 2026-11-12, последний доступный upstream. SOURCE.json содержит SHA256 файлов. CurrentSchema.h удален: offsets.h ссылается прямо на актуальные заголовки.

Направление взгляда: C_CSPlayerPawn::m_angEyeAngles (0x3630), pitch/yaw в градусах -> forward vector. Линия от origin+viewOffset, настройки View direction и Direction length. Это сетевые углы pawn, не гарантированное точное положение камеры другого клиента.

ESP: controller -> m_hPlayerPawn (0x92C) -> страница entity list -> pawn. Чтение health/team/origin/angles с pawn; имя с controller. Общий resolver handles учитывает границы страниц, invalid/null handles. Основной box строится от origin до глаз и не требует костей; skeleton остается отдельной функцией. Индексная маска, шаг записи и страницы вынесены в EntityLayout; эти значения не публикуются cs2-dumper и не выдаются за автоматически обновленные поля схемы.

Тесты направления взгляда проверяют три оси, NaN, нулевую длину. Проверка в игре не выполнена.

Дополнительно по локальной client.dll: шаг entity identity 0x70 (40 совпадений AND 0x1FF; IMUL 0x70), полный handle сравнивается по identity+0x10. EntityList pattern заменен на upstream: единственное совпадение RVA 0x15B5D39 -> 0x2709010. Предыдущий паттерн указывал на другой глобальный адрес.
