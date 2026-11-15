> обновление 2026-11-15: устранён блокирующий отказ обязательной загрузки material/CRC функций. см. STARTUP_FIX_RU.md; ниже сохранён первоначальный аудит. актуальный совокупный diff находится в changes.patch.

# Ревизия Asphyxia из приложенного архива

kindmops, это статическая ревизия исходников, исправленная копия и кандидаты миграции, **не подтверждённая поддержка текущей локальной сборки CS2**.

Дата проверки: 2026-11-14. В архиве `CS_PRODUCTSTRINGVERSION="1.40.5.5"`, но это не надёжный идентификатор всех файлов архива. Публичный [a2x info.json](https://github.com/a2x/cs2-dumper/blob/main/output/info.json) сообщает build **14197**, timestamp **2026-11-12T13:07:45.363725600+00:00**. Копии источников и SHA-256 лежат в `evidence/`. Локальных игровых DLL нет.

Сначала для каждого файла приведены исходные литералы/сигнатуры, затем выводы и точный diff. Числа из комментариев явно помечены; это не вычисленные runtime-offsets. Неизменённые файлы также разобраны. Полные изменённые файлы лежат в дереве проекта, а все изменения одним файлом — `changes.patch`.

## Проверяемые изменения из публичного снимка

| Элемент | Исходник | Публичное значение / кандидат | Статус |
|---|---|---|---|
| highestEntityIndex | +0x1510 | +0x2120; извлекается из FF 81 disp32 48 85 D2 | внешний снимок, не local-test |
| dwCSGOInput | фиксированный RIP pattern | client RVA 0x2569E30; новый pattern ниже | внешний снимок |
| dwGlobalVars | фиксированный RIP pattern | client RVA 0x2221AB8; новый pattern ниже | внешний снимок |
| dwGameEntitySystem | GameResourceService+0x58 | client RVA 0x2709010 | альтернативный storage, не доказательство +0x58 |
| dwViewRender | только hook argument | client RVA 0x255A008 | не доказательство argument layout |
| m_bIsFullAuto | CBasePlayerWeaponVData | CCSWeaponBaseVData, schema offset0x72D | исправлен owner schema key |
| CreateMove / UserCmd / CRC | legacy declarations | новые wire fields, memory ABI неизвестен | legacy hook по умолчанию выключен |

Источники: [offsets](https://github.com/a2x/cs2-dumper/blob/main/output/offsets.json), [алгоритмы поиска](https://github.com/a2x/cs2-dumper/blob/main/src/analysis/offsets.rs), [интерфейсы](https://github.com/a2x/cs2-dumper/blob/main/output/interfaces.json), [schema](https://github.com/a2x/cs2-dumper/blob/main/output/client_dll.json), [usercmd.proto](https://github.com/SteamDatabase/GameTracking-CS2/blob/master/Protobufs/usercmd.proto), [cs_usercmd.proto](https://github.com/SteamDatabase/GameTracking-CS2/blob/master/Protobufs/cs_usercmd.proto).

## Полный реестр FindPattern

29 call-sites; одинаковый GlobalVars встречается дважды. В schemasystem.dll и inputsystem.dll в оригинале нет FindPattern. Эти интерфейсы получались через CreateInterface registration list. Найдены также обязательные для материалов/рендера particles.dll и scenesystem.dll.

| ID | Файл:строка | Модуль | Исходный IDA pattern | Relative decode |
|---|---|---|---|---|
| P01 | `cstrike/core/hooks.cpp:83` | client.dll | `40 53 48 81 EC ? ? ? ? 49 8B C1` | нет |
| P02 | `cstrike/core/hooks.cpp:92` | client.dll | `48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 56 57 48 8D A8` | нет |
| P03 | `cstrike/core/hooks.cpp:109` | client.dll | `48 89 5C 24 ? 56 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B F2` | нет |
| P04 | `cstrike/core/hooks.cpp:114` | client.dll | `48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4 28 E9 C3 20 01 ?` | нет |
| P05 | `cstrike/core/hooks.cpp:126` | client.dll | `48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8 20 1E ED FF` | нет |
| P06 | `cstrike/core/hooks.cpp:132` | scenesystem.dll | `48 8B C4 48 89 50 ? 53` | нет |
| P07 | `cstrike/core/hooks.cpp:270` | client.dll | `48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01` | disp+3, end+7 |
| P08 | `cstrike/core/interfaces.cpp:155` | rendersystemdx11.dll | `66 0F 7F 0D 83 C9 43 ? 48 8B F7 66 0F 7F 05 88 C9 43 ?` | disp+4, end+8 |
| P09 | `cstrike/core/interfaces.cpp:174` | client.dll | `48 8B 0D D1 3A 29 01 4C 8D 8F E0 05 ? ? 45 33 FF` | disp+3, end+7 |
| P10 | `cstrike/core/interfaces.cpp:179` | client.dll | `48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01` | disp+3, end+7 |
| P11 | `cstrike/core/interfaces.cpp:183` | engine2.dll | `48 8D 0D ? ? ? ? 33 ? FF 50` | disp+3, end+7 |
| P12 | `cstrike/core/interfaces.cpp:188` | client.dll | `48 8B 1D ? ? ? ? 24 ? 0C ? F3 0F 7F 45` | disp+3, end+7 |
| P13 | `cstrike/features/CRC.h:60` | client.dll | `48 89 5C 24 18 55 56 57 48 81 EC 90` | нет |
| P14 | `cstrike/features/CRC.h:75` | client.dll | `48 89 5C 24 10 48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 BF` | нет |
| P15 | `cstrike/features/CRC.h:82` | client.dll | `48 89 5C 24 20 55 56 57 48 83 EC 30 49` | нет |
| P16 | `cstrike/sdk/datatypes/keyvalue3.h:26` | client.dll | `40 53 48 83 EC 20 48 8B 01 48 8B D9 44` | нет |
| P17 | `cstrike/sdk/datatypes/keyvalues3.cpp:17` | tier0.dll | `E8 ? ? ? ? EB 36 8B 43 10` | disp+1, end+5 |
| P18 | `cstrike/sdk/datatypes/usercmd.h:118` | client.dll | `E8 ? ? ? ? 4C 8B E0 48 8B 44 24 ? 4C 8B CF` | disp+1, end+5 |
| P19 | `cstrike/sdk/datatypes/usercmd.h:234` | client.dll | `E8 ? ? ? ? 48 8B D0 48 8D 4F 18 E8 ? ? ? ? 48 8B D0` | disp+1, end+5 |
| P20 | `cstrike/sdk/interfaces/ccsgoinput.h:75` | client.dll | `85 D2 75 3F 48` | нет |
| P21 | `cstrike/sdk/interfaces/ccsgoinput.h:87` | client.dll | `4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3` | нет |
| P22 | `cstrike/sdk/interfaces/cgameentitysystem.h:45` | client.dll | `81 FA ? ? ? ? 77 ? 8B C2 C1 F8 ? 83 F8 ? 77 ? 48 98 48 8B 4C C1 ? 48 85 C9 74 ? 8B C2 25 ? ? ? ? 48 6B C0 ? 48 03 C8 74 ? 8B 41 ? 25 ? ? ? ? 3B C2 75 ? 48 8B 01` | нет |
| P23 | `cstrike/sdk/interfaces/cgametracemanager.cpp:9` | client.dll | `E8 ? ? ? ? 48 85 C0 74 ? 44 38 60` | disp+1, end+5 |
| P24 | `cstrike/sdk/interfaces/cgametracemanager.h:98` | client.dll | `48 89 5C 24 20 48 89 4C 24 08 55 56 41` | нет |
| P25 | `cstrike/sdk/interfaces/cgametracemanager.h:110` | client.dll | `48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C` | нет |
| P26 | `cstrike/sdk/interfaces/imaterialsystem.h:43` | particles.dll | `48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA` | нет |
| P27 | `cstrike/sdk/interfaces/imaterialsystem.h:74` | particles.dll | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 4D 8B E0 4C 8B 72 ? 48 8B F9 C0 E8 ? 24 ? 3C ? 74 ? 41 B0 ? B2 ? E8 ? ? ? ? 0F B6 07 33 DB C0 E8 ? 24 ? 3C ? 75 ? 48 8B 77 ? EB ? 48 8B F3 4C 8D 44 24 ? C7 44 24 ? ? ? ? ? 48 8D 54 24 ? 89 6C 24 ? 48 8B CE 4C 89 74 24 ? E8 ? ? ? ? 8B D0 83 F8 ? 75 ? 45 33 C9 89 6C 24 ? 4C 8D 44 24 ? 4C 89 74 24 ? 48 8B D7 48 8B CE E8 ? ? ? ? 8B D0 0F B6 0F C0 E9 ? 80 E1 ? 80 F9 ? 75 ? 48 8B 4F ? EB ? 48 8B CB 8B 41 ? 85 C0 74 ? 48 8D 59 ? 83 F8 ? 76 ? 48 8B 1B 48 63 C2 4D 85 E4` | нет |
| P28 | `cstrike/sdk/interfaces/imaterialsystem.h:87` | particles.dll | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 48 8B F9` | нет |
| P29 | `cstrike/utilities/memory.cpp:30` | materialsystem2.dll | `48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05` | нет |

`ResolveRelativeAddress(CreateInterface,3,7)` не содержит FindPattern и отдельно учтён в interfaces.cpp. `GetAbsoluteAddress` и RIP resolution показывают адрес цели инструкции, а не автоматически объект/функцию: глубина чтения указателей проверяется отдельно.

## Layout команд, вычисляемый из старых объявлений x64

| Тип | Поля / смещения | sizeof старого объявления |
|---|---|---|
| CBasePB | pad0..7; nHasBits8; paddingC..F; nCachedBits10 | 0x18 |
| CInButtonState | pad0..7; value8/changed10/scroll18 | 0x20 |
| CInButtonStatePB | base18; value18/changed20/scroll28 | 0x30 |
| CSubtickMoveStep | button18; pressed20; when24; forward28; left2C | 0x30 |
| CBaseUserCmdPB | repeated18; crc30; buttons38; angles40; command48; tick4C; forward50; side54; up58; impulse5C; weapon60; seed64; mouse68/6C; consumed70; flags74; handle78 | 0x80 |
| CCSGOUserCmdPB | has0; cached8; history10; base28; left30; attack3=34/attack1=38/attack2=3C | 0x40 |
| CUserCmd | csgoUserCmd18; buttons58; tail78 | 0x98 |

Эта таблица проверяет согласованность деклараций, не текущую память игры. .proto задаёт field numbers/types, но не C++ offsets, размер repeated-field, _has_bits_ или ABI строки.

В новых wire definitions CSubtickMoveStep содержит pitch_delta=8/yaw_delta=9. CBaseUserCmdPB содержит prediction_offset_ticks_x256=17 и execution_notes=22. CSGOUserCmdPB не содержит attack3_start_history_index, зато содержит is_predicting_body_shot_fx=11, is_predicting_head_shot_fx=12 и is_predicting_kill_ragdolls=13. Поэтому старые sizeof/asserts недостаточны.

## `cstrike/core/interfaces.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 34 | экспорт | `std::uint8_t* pCreateInterface = reinterpret_cast<std::uint8_t*>(MEM::GetExportAddress(hModule, CS_XOR("CreateInterface")));` |
| 42 | relative decode | `return *reinterpret_cast<CInterfaceRegister**>(MEM::ResolveRelativeAddress(pCreateInterface, 0x3, 0x7));` |
| 87 | экспорт | `MemAlloc = *reinterpret_cast<IMemAlloc**>(MEM::GetExportAddress(pTier0Handle, CS_XOR("g_pMemAlloc")));` |
| 155 | IDA pattern | `SwapChain = **reinterpret_cast<ISwapChainDx11***>(MEM::ResolveRelativeAddress(MEM::FindPattern(RENDERSYSTEM_DLL, CS_XOR("66 0F 7F 0D 83 C9 43 ? 48 8B F7 66 0F 7F 05 88 C9 43 ?")), 0x4, 0x8));` |
| 174 | IDA pattern | `Input = *reinterpret_cast<CCSGOInput**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D D1 3A 29 01 4C 8D 8F E0 05 ? ? 45 33 FF")), 0x3, 0x7));` |
| 179 | IDA pattern | `GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01")), 0x3, 0x7));` |
| 183 | IDA pattern | `PVS = reinterpret_cast<CPVS*>(MEM::ResolveRelativeAddress(MEM::FindPattern(ENGINE2_DLL, CS_XOR("48 8D 0D ? ? ? ? 33 ? FF 50")), 0x3, 0x7));` |
| 188 | IDA pattern | `GameTraceManager = *reinterpret_cast<CGameTraceManager**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 1D ? ? ? ? 24 ? 0C ? F3 0F 7F 45")), 0x3, 0x0));` |

Извлечение registration list по первым 7 байтам CreateInterface заменено прямым вызовом экспортированной фабрики с точной версией интерфейса. Это устраняет предположение о прологе `+3/+7`. Имена взяты из публичного interfaces.json, не из валидации VTable.

Input заменён на `48 89 05 ? ? ? ? 0F 57 C0 0F 11 05`, GlobalVars — на `48 89 15 ? ? ? ? 48 89 42`; оба используют signed disp32 с offset 3/end 7 и одно чтение указателя. Старые строки включали фиксированные байты относительных адресов.

SwapChain пока остаётся неподтверждённым: фиксированные байты `83 C9 43` / `88 C9 43`, два разыменования и wrapper+0x170 требуют новой проверки. Стартовый opcode — SIMD-store, не доказательство pointer-storage. Добавлены проверки адресов при чтении указателей. PVS и GameTraceManager не объявлены актуальными.

CGameEntitySystem в оригинале получается через GameResourceService+0x58, а не через отдельную глобальную сигнатуру. IViewRender не захватывается глобально: приходит параметром GetMatrixForView. Повторное создание RTV больше не накапливает ссылки GetDevice/GetImmediateContext.

Изменённый код (unified diff):

```diff
--- a/cstrike/core/interfaces.cpp
+++ b/cstrike/core/interfaces.cpp
@@ -15,62 +15,19 @@
 
 #pragma region interfaces_get
 
-using InstantiateInterfaceFn_t = void* (*)();
-
-class CInterfaceRegister
-{
-public:
-	InstantiateInterfaceFn_t fnCreate;
-	const char* szName;
-	CInterfaceRegister* pNext;
-};
-
-static const CInterfaceRegister* GetRegisterList(const wchar_t* wszModuleName)
-{
-	void* hModule = MEM::GetModuleBaseHandle(wszModuleName);
-	if (hModule == nullptr)
-		return nullptr;
-
-	std::uint8_t* pCreateInterface = reinterpret_cast<std::uint8_t*>(MEM::GetExportAddress(hModule, CS_XOR("CreateInterface")));
-
-	if (pCreateInterface == nullptr)
-	{
-		L_PRINT(LOG_ERROR) << CS_XOR("failed to get \"CreateInterface\" address");
-		return nullptr;
-	}
-
-	return *reinterpret_cast<CInterfaceRegister**>(MEM::ResolveRelativeAddress(pCreateInterface, 0x3, 0x7));
-}
-
-template <typename T = void*>
-T* Capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
-{
-	for (const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext)
-	{
-		if (const std::size_t nInterfaceNameLength = CRT::StringLength(szInterfaceName);
-			// found needed interface
-			CRT::StringCompareN(szInterfaceName, pRegister->szName, nInterfaceNameLength) == 0 &&
-			// and we've given full name with hardcoded digits
-			(CRT::StringLength(pRegister->szName) == nInterfaceNameLength ||
-			// or it contains digits after name
-			CRT::StringToInteger<int>(pRegister->szName + nInterfaceNameLength, nullptr, 10) > 0))
-		{
-			// capture our interface
-			void* pInterface = pRegister->fnCreate();
-
-#ifdef _DEBUG
-			// log interface address
-			L_PRINT(LOG_INFO) << CS_XOR("captured \"") << pRegister->szName << CS_XOR("\" interface at address: ") << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<std::uintptr_t>(pInterface);
-#else
-			L_PRINT(LOG_INFO) << CS_XOR("captured \"") << pRegister->szName << CS_XOR("\" interface");
-#endif
-
-			return static_cast<T*>(pInterface);
-		}
-	}
-
-	L_PRINT(LOG_ERROR) << CS_XOR("failed to find interface \"") << szInterfaceName << CS_XOR("\"");
-	return nullptr;
+// Query the exported factory with an exact interface version. No CreateInterface opcode assumptions.
+static const void* GetFactoryModule(const wchar_t* module)
+{
+    return MEM::GetModuleBaseHandle(module);
+}
+
+template <typename T>
+T* Capture(const void* module, const char* name)
+{
+    if (!module) return nullptr;
+    using Factory = void*(CS_CDECL*)(const char*, int*);
+    auto factory = reinterpret_cast<Factory>(MEM::GetExportAddress(module, CS_XOR("CreateInterface")));
+    return factory ? static_cast<T*>(factory(name, nullptr)) : nullptr;
 }
 
 #pragma endregion
@@ -84,24 +41,24 @@
 	if (pTier0Handle == nullptr)
 		return false;
 
-	MemAlloc = *reinterpret_cast<IMemAlloc**>(MEM::GetExportAddress(pTier0Handle, CS_XOR("g_pMemAlloc")));
+	MemAlloc = MEM::ReadPointer<IMemAlloc>(MEM::GetExportAddress(pTier0Handle, CS_XOR("g_pMemAlloc")));
 	bSuccess &= (MemAlloc != nullptr);
 
-	const auto pSchemaSystemRegisterList = GetRegisterList(SCHEMASYSTEM_DLL);
+	const auto pSchemaSystemRegisterList = GetFactoryModule(SCHEMASYSTEM_DLL);
 	if (pSchemaSystemRegisterList == nullptr)
 		return false;
 
 	SchemaSystem = Capture<ISchemaSystem>(pSchemaSystemRegisterList, SCHEMA_SYSTEM);
 	bSuccess &= (SchemaSystem != nullptr);
 
-	const auto pInputSystemRegisterList = GetRegisterList(INPUTSYSTEM_DLL);
+	const auto pInputSystemRegisterList = GetFactoryModule(INPUTSYSTEM_DLL);
 	if (pInputSystemRegisterList == nullptr)
 		return false;
 
 	InputSystem = Capture<IInputSystem>(pInputSystemRegisterList, INPUT_SYSTEM_VERSION);
 	bSuccess &= (InputSystem != nullptr);
 
-	const auto pEngineRegisterList = GetRegisterList(ENGINE2_DLL);
+	const auto pEngineRegisterList = GetFactoryModule(ENGINE2_DLL);
 	if (pEngineRegisterList == nullptr)
 		return false;
 
@@ -114,28 +71,28 @@
 	NetworkClientService = Capture<INetworkClientService>(pEngineRegisterList, NETWORK_CLIENT_SERVICE);
 	bSuccess &= (NetworkClientService != nullptr);
 
-	const auto pTier0RegisterList = GetRegisterList(TIER0_DLL);
+	const auto pTier0RegisterList = GetFactoryModule(TIER0_DLL);
 	if (pTier0RegisterList == nullptr)
 		return false;
 
 	Cvar = Capture<IEngineCVar>(pTier0RegisterList, ENGINE_CVAR);
 	bSuccess &= (Cvar != nullptr);
 
-	const auto pClientRegister = GetRegisterList(CLIENT_DLL);
+	const auto pClientRegister = GetFactoryModule(CLIENT_DLL);
 	if (pClientRegister == nullptr)
 		return false;
 
 	Client = Capture<ISource2Client>(pClientRegister, SOURCE2_CLIENT);
 	bSuccess &= (Client != nullptr);
 
-	const auto pMaterialSystem2Register = GetRegisterList(MATERIAL_SYSTEM2_DLL);
+	const auto pMaterialSystem2Register = GetFactoryModule(MATERIAL_SYSTEM2_DLL);
 	if (pMaterialSystem2Register == nullptr)
 		return false;
 
 	MaterialSystem2 = Capture<IMaterialSystem2>(pMaterialSystem2Register, MATERIAL_SYSTEM2);
 	bSuccess &= (MaterialSystem2 != nullptr);
 
-	const auto pResourceSystemRegisterList = GetRegisterList(RESOURCESYSTEM_DLL);
+	const auto pResourceSystemRegisterList = GetFactoryModule(RESOURCESYSTEM_DLL);
 	if (pResourceSystemRegisterList == nullptr)
 		return false;
 
@@ -152,11 +109,11 @@
 
 	// @ida:  #STR: "r_gpu_mem_stats", "-threads", "CTSListBase: Misaligned list\n", "CTSQueue: Misaligned queue\n", "Display GPU memory usage.", "-r_max_device_threads"
 	// https://media.discordapp.net/attachments/1055004763328106558/1315619131109937152/image.png?ex=6758114b&is=6756bfcb&hm=a568636a5292e2a04f94972f5781d8ad88f170a38ec2a1ce82135726dec23fac&=&format=webp&quality=lossless
-	SwapChain = **reinterpret_cast<ISwapChainDx11***>(MEM::ResolveRelativeAddress(MEM::FindPattern(RENDERSYSTEM_DLL, CS_XOR("66 0F 7F 0D 83 C9 43 ? 48 8B F7 66 0F 7F 05 88 C9 43 ?")), 0x4, 0x8));
+	SwapChain = MEM::ReadPointer<ISwapChainDx11>(MEM::ReadPointer<ISwapChainDx11*>(MEM::ResolveRelativeAddress(MEM::FindPattern(RENDERSYSTEM_DLL, CS_XOR("66 0F 7F 0D 83 C9 43 ? 48 8B F7 66 0F 7F 05 88 C9 43 ?")), 0x4, 0x8)));
 	bSuccess &= (SwapChain != nullptr);
 
 	// grab's d3d11 interfaces for later use
-	if (SwapChain != nullptr)
+	if (SwapChain != nullptr && SwapChain->pDXGISwapChain != nullptr)
 	{
 		if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
 		{
@@ -171,12 +128,12 @@
 	bSuccess &= (Device != nullptr && DeviceContext != nullptr);
 	
 	// #STR: CSGOInput
-	Input = *reinterpret_cast<CCSGOInput**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D D1 3A 29 01 4C 8D 8F E0 05 ? ? 45 33 FF")), 0x3, 0x7));
+	Input = MEM::ReadPointer<CCSGOInput>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 05 ? ? ? ? 0F 57 C0 0F 11 05")), 0x3, 0x7));
 	bSuccess &= (Input != nullptr);
 
 	// @ida: #STR: "gpGlocals->rendertime() called while IsInSimulation() is true, "gpGlocals->curtime() called while IsInSimulation() is false
 	// @ida: #STR: "C_SceneEntity::SetupClientOnlyScene:  C" then go up until you see it
-	GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01")), 0x3, 0x7));
+	GlobalVars = MEM::ReadPointer<IGlobalVars>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 15 ? ? ? ? 48 89 42")), 0x3, 0x7));
 	bSuccess &= (GlobalVars != nullptr);
 
 	// @ida: #STR: "CRenderingWorldSession::OnLoopActivate" go down just a bit
@@ -185,7 +142,7 @@
 
 	// @ida: #STR: "Physics/TraceShape (Client)"
 	// @ida: #STR: "Weapon_Knife.Stab" then go up
-	GameTraceManager = *reinterpret_cast<CGameTraceManager**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 1D ? ? ? ? 24 ? 0C ? F3 0F 7F 45")), 0x3, 0x0));
+	GameTraceManager = MEM::ReadPointer<CGameTraceManager>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 1D ? ? ? ? 24 ? 0C ? F3 0F 7F 45")), 0x3, 0x0));
 	bSuccess &= (GameTraceManager != nullptr);
 
 	return bSuccess;
@@ -193,15 +150,8 @@
 
 void I::CreateRenderTarget()
 {
-	if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
-	{
-		L_PRINT(LOG_ERROR) << CS_XOR("failed to get device from swapchain");
-		CS_ASSERT(false);
-	}
-	else
-		// we successfully got device, so we can get immediate context
-		Device->GetImmediateContext(&DeviceContext);
-
+    if (!SwapChain || !SwapChain->pDXGISwapChain || !Device) return;
+    DestroyRenderTarget();
 	// @note: i dont use this anywhere else so lambda is fine
 	static const auto GetCorrectDXGIFormat = [](DXGI_FORMAT eCurrentFormat)
 	{
@@ -214,8 +164,8 @@
 		return eCurrentFormat;
 	};
 
-	DXGI_SWAP_CHAIN_DESC sd;
-	SwapChain->pDXGISwapChain->GetDesc(&sd);
+	DXGI_SWAP_CHAIN_DESC sd{};
+    if (FAILED(SwapChain->pDXGISwapChain->GetDesc(&sd))) return;
 
 	ID3D11Texture2D* pBackBuffer = nullptr;
 	if (SUCCEEDED(SwapChain->pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
```

## `cstrike/utilities/memory.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 24 | экспорт | `fnUnDecorateSymbolName = reinterpret_cast<decltype(fnUnDecorateSymbolName)>(GetExportAddress(hDbgHelp, CS_XOR("UnDecorateSymbolName")));` |
| 27 | экспорт | `fnLoadKV3 = reinterpret_cast<decltype(fnLoadKV3)>(GetExportAddress(hTier0, CS_XOR("?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2@Z")));` |
| 30 | IDA pattern | `fnCreateMaterial = reinterpret_cast<decltype(fnCreateMaterial)>(FindPattern(MATERIAL_SYSTEM2_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05")));` |
| 33 | экспорт | `fnUtlBufferInit = reinterpret_cast<decltype(fnUtlBufferInit)>(GetExportAddress(hTier0, CS_XOR("??0CUtlBuffer@@QEAA@HHH@Z")));` |
| 36 | экспорт | `fnUtlBufferPutString = reinterpret_cast<decltype(fnUtlBufferPutString)>(GetExportAddress(hTier0, CS_XOR("?PutString@CUtlBuffer@@QEAAXPEBD@Z")));` |
| 39 | экспорт | `fnUtlBufferEnsureCapacity = reinterpret_cast<decltype(fnUtlBufferEnsureCapacity)>(GetExportAddress(hTier0, CS_XOR("?EnsureCapacity@CUtlBuffer@@QEAAXH@Z")));` |
| 108 | offset / размер / mask / layout | `const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));` |
| 133 | offset / размер / mask / layout | `const _PEB* pPEB = reinterpret_cast<_PEB*>(__readgsqword(0x60));` |
| 157 | экспорт | `void* MEM::GetExportAddress(const void* hModuleBase, const char* szProcedureName)` |

Исправлены пропуск последней позиции, underflow при pattern > region, пустой pattern и неправильные перескоки после частичного совпадения. Старые перескоки могли пропустить перекрывающиеся совпадения. Parser больше не выходит за NUL на конечном `?`; поддерживает `??`, whitespace и отклоняет некорректный hex.

Поиск модульных паттернов теперь выполняется в читаемых executable-секциях и требует ровно одного совпадения; debug-режим оригинала лишь предупреждал о неоднозначности и всё равно выбирал первое. Это может обнаружить неоднозначность старых коротких паттернов вместо скрытого выбора адреса. Добавлен null guard GetExportAddress. CreateMaterial pattern не обновлён без тела DLL.

LoadKV3 export уже был в исходнике: новая реализация keyvalues3.cpp использует его, а не второй непроверенный tier0 pattern.

Изменённый код (unified diff):

```diff
--- a/cstrike/utilities/memory.cpp
+++ b/cstrike/utilities/memory.cpp
@@ -11,6 +11,30 @@
 #include "crt.h"
 // used: pe64
 #include "pe64.h"
+#include <algorithm>
+#include <limits>
+
+bool MEM::IsReadable(const void* address, std::size_t bytes)
+{
+    auto cursor = reinterpret_cast<std::uintptr_t>(address);
+    if (!cursor || !bytes || bytes > (std::numeric_limits<std::uintptr_t>::max)() - cursor)
+        return false;
+    const auto end = cursor + bytes;
+    while (cursor < end)
+    {
+        MEMORY_BASIC_INFORMATION region{};
+        if (!::VirtualQuery(reinterpret_cast<const void*>(cursor), &region, sizeof(region)) ||
+            region.State != MEM_COMMIT || (region.Protect & PAGE_GUARD)) return false;
+        const auto access = region.Protect & 0xFF;
+        if (access != PAGE_READONLY && access != PAGE_READWRITE && access != PAGE_WRITECOPY &&
+            access != PAGE_EXECUTE_READ && access != PAGE_EXECUTE_READWRITE && access != PAGE_EXECUTE_WRITECOPY)
+            return false;
+        const auto regionEnd = reinterpret_cast<std::uintptr_t>(region.BaseAddress) + region.RegionSize;
+        if (regionEnd <= cursor) return false;
+        cursor = (std::min)(end, regionEnd);
+    }
+    return true;
+}
 
 bool MEM::Setup()
 {
@@ -156,6 +180,7 @@
 
 void* MEM::GetExportAddress(const void* hModuleBase, const char* szProcedureName)
 {
+    if (!hModuleBase || !szProcedureName) return nullptr;
 	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);
 
 	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
@@ -261,227 +286,77 @@
 
 std::uint8_t* MEM::FindPattern(const wchar_t* wszModuleName, const char* szPattern)
 {
-	// convert pattern string to byte array
-	const std::size_t nApproximateBufferSize = (CRT::StringLength(szPattern) >> 1U) + 1U;
-	std::uint8_t* arrByteBuffer = static_cast<std::uint8_t*>(MEM_STACKALLOC(nApproximateBufferSize));
-	char* szMaskBuffer = static_cast<char*>(MEM_STACKALLOC(nApproximateBufferSize));
-	PatternToBytes(szPattern, arrByteBuffer, szMaskBuffer);
-
-	// @test: use search with straight in-place conversion? do not think it will be faster, cuz of bunch of new checks that gonna be performed for each iteration
-	return FindPattern(wszModuleName, reinterpret_cast<const char*>(arrByteBuffer), szMaskBuffer);
+    if (!szPattern) return nullptr;
+    const auto capacity = CRT::StringLength(szPattern) + 1;
+    std::vector<std::uint8_t> bytes(capacity);
+    std::vector<char> mask(capacity);
+    if (!PatternToBytes(szPattern, bytes.data(), mask.data())) return nullptr;
+    return FindPattern(wszModuleName, reinterpret_cast<const char*>(bytes.data()), mask.data());
 }
 
 std::uint8_t* MEM::FindPattern(const wchar_t* wszModuleName, const char* szBytePattern, const char* szByteMask)
 {
-	const void* hModuleBase = GetModuleBaseHandle(wszModuleName);
-
-	if (hModuleBase == nullptr)
-	{
-		L_PRINT(LOG_ERROR) << CS_XOR("failed to get module handle for: \"") << wszModuleName << CS_XOR("\"");
-		return nullptr;
-	}
-
-	const auto pBaseAddress = static_cast<const std::uint8_t*>(hModuleBase);
-
-	const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
-	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
-	{
-		L_PRINT(LOG_ERROR) << CS_XOR("failed to get module size, image is invalid");
-		return nullptr;
-	}
-
-	const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS*>(pBaseAddress + pIDH->e_lfanew);
-	if (pINH->Signature != IMAGE_NT_SIGNATURE)
-	{
-		L_PRINT(LOG_ERROR) << CS_XOR("failed to get module size, image is invalid");
-		return nullptr;
-	}
-
-	const std::uint8_t* arrByteBuffer = reinterpret_cast<const std::uint8_t*>(szBytePattern);
-	const std::size_t nByteCount = CRT::StringLength(szByteMask);
-
-	std::uint8_t* pFoundAddress = nullptr;
-
-	// perform little overhead to keep all patterns unique
-#ifdef CS_PARANOID_PATTERN_UNIQUENESS
-	const std::vector<std::uint8_t*> vecFoundOccurrences = FindPatternAllOccurrencesEx(pBaseAddress, pINH->OptionalHeader.SizeOfImage, arrByteBuffer, nByteCount, szByteMask);
-
-	// notify user about non-unique pattern
-	if (!vecFoundOccurrences.empty())
-	{
-		// notify user about non-unique pattern
-		if (vecFoundOccurrences.size() > 1U)
-		{
-			char* szPattern = static_cast<char*>(MEM_STACKALLOC((nByteCount << 1U) + nByteCount));
-			[[maybe_unused]] const std::size_t nConvertedPatternLength = BytesToPattern(arrByteBuffer, nByteCount, szPattern);
-
-			L_PRINT(LOG_WARNING) << CS_XOR("found more than one occurrence with \"") << szPattern << CS_XOR("\" pattern, consider updating it!");
-
-			MEM_STACKFREE(szPattern);
-		}
-
-		// return first found occurrence
-		pFoundAddress = vecFoundOccurrences[0];
-	}
-#else
-	// @todo: we also can go through code sections and skip noexec pages, but will it really improve performance? / or at least for all occurrences search
-	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header
-#if 0
-	IMAGE_SECTION_HEADER* pCurrentSection = IMAGE_FIRST_SECTION(pINH);
-	for (WORD i = 0U; i != pINH->FileHeader.NumberOfSections; i++)
-	{
-		// check does page have executable code
-		if (pCurrentSection->Characteristics & IMAGE_SCN_CNT_CODE || pCurrentSection->Characteristics & IMAGE_SCN_MEM_EXECUTE)
-		{
-			pFoundAddress = FindPatternEx(pBaseAddress + pCurrentSection->VirtualAddress, pCurrentSection->SizeOfRawData, arrByteBuffer, nByteCount, szByteMask);
-
-			if (pFoundAddress != nullptr)
-				break;
-		}
-
-		++pCurrentSection;
-	}
-#else
-	pFoundAddress = FindPatternEx(pBaseAddress, pINH->OptionalHeader.SizeOfImage, arrByteBuffer, nByteCount, szByteMask);
-#endif
-#endif
-
-	if (pFoundAddress == nullptr)
-	{
-		char* szPattern = static_cast<char*>(MEM_STACKALLOC((nByteCount << 1U) + nByteCount));
-		[[maybe_unused]] const std::size_t nConvertedPatternLength = BytesToPattern(arrByteBuffer, nByteCount, szPattern);
-
-		L_PRINT(LOG_ERROR) << CS_XOR("pattern not found: \"") << szPattern << CS_XOR("\"");
-
-		MEM_STACKFREE(szPattern);
-	}
-
-	return pFoundAddress;
+    if (!szBytePattern || !szByteMask || !*szByteMask) return nullptr;
+    const auto base = static_cast<const std::uint8_t*>(GetModuleBaseHandle(wszModuleName));
+    if (!IsReadable(base, sizeof(IMAGE_DOS_HEADER))) return nullptr;
+    const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
+    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) return nullptr;
+    const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
+    if (!IsReadable(nt, sizeof(*nt)) || nt->Signature != IMAGE_NT_SIGNATURE ||
+        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return nullptr;
+    const auto sections = IMAGE_FIRST_SECTION(nt);
+    if (!IsReadable(sections, sizeof(*sections) * nt->FileHeader.NumberOfSections)) return nullptr;
+    const auto count = CRT::StringLength(szByteMask);
+    std::uint8_t* result = nullptr;
+    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i)
+    {
+        const auto& section = sections[i];
+        if (!(section.Characteristics & IMAGE_SCN_MEM_EXECUTE) || section.VirtualAddress >= nt->OptionalHeader.SizeOfImage)
+            continue;
+        const auto size = (std::min)(section.Misc.VirtualSize,
+            nt->OptionalHeader.SizeOfImage - section.VirtualAddress);
+        const auto start = base + section.VirtualAddress;
+        if (!IsReadable(start, size)) continue;
+        const auto matches = FindPatternAllOccurrencesEx(start, size,
+            reinterpret_cast<const std::uint8_t*>(szBytePattern), count, szByteMask);
+        if (matches.size() > 1 || (result && !matches.empty()))
+        {
+            L_PRINT(LOG_ERROR) << CS_XOR("ambiguous executable pattern; refusing first-match binding");
+            return nullptr;
+        }
+        if (!matches.empty()) result = matches.front();
+    }
+    if (!result) L_PRINT(LOG_ERROR) << CS_XOR("executable pattern not found");
+    return result;
 }
 
 // @todo: msvc poorly optimizes this, it looks even better w/o optimization at all
-std::uint8_t* MEM::FindPatternEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask)
-{
-	std::uint8_t* pCurrentAddress = const_cast<std::uint8_t*>(pRegionStart);
-	const std::uint8_t* pRegionEnd = pRegionStart + nRegionSize - nByteCount;
-	const bool bIsMaskUsed = (szByteMask != nullptr);
-
-	while (pCurrentAddress < pRegionEnd)
-	{
-		// check the first byte before entering the loop, otherwise if there two consecutive bytes of first byte in the buffer, we may skip both and fail the search
-		if ((bIsMaskUsed && *szByteMask == '?') || *pCurrentAddress == *arrByteBuffer)
-		{
-			if (nByteCount == 1)
-				return pCurrentAddress;
-
-			// compare the least byte sequence and continue on wildcard or skip forward on first mismatched byte
-			std::size_t nComparedBytes = 0U;
-			while ((bIsMaskUsed && szByteMask[nComparedBytes + 1U] == '?') || pCurrentAddress[nComparedBytes + 1U] == arrByteBuffer[nComparedBytes + 1U])
-			{
-				// check does byte sequence match
-				if (++nComparedBytes == nByteCount - 1U)
-					return pCurrentAddress;
-			}
-
-			// skip non suitable bytes
-			pCurrentAddress += nComparedBytes;
-		}
-
-		++pCurrentAddress;
-	}
-
-	return nullptr;
-}
-
-std::vector<std::uint8_t*> MEM::FindPatternAllOccurrencesEx(const std::uint8_t* pRegionStart, const std::size_t nRegionSize, const std::uint8_t* arrByteBuffer, const std::size_t nByteCount, const char* szByteMask)
-{
-	const std::uint8_t* pRegionEnd = pRegionStart + nRegionSize - nByteCount;
-	const bool bIsMaskUsed = (szByteMask != nullptr);
-
-	// container for addresses of the all found occurrences
-	std::vector<std::uint8_t*> vecOccurrences = {};
-
-	for (std::uint8_t* pCurrentByte = const_cast<std::uint8_t*>(pRegionStart); pCurrentByte < pRegionEnd; ++pCurrentByte)
-	{
-		// do a first byte check before entering the loop, otherwise if there two consecutive bytes of first byte in the buffer, we may skip both and fail the search
-		if ((!bIsMaskUsed || *szByteMask != '?') && *pCurrentByte != *arrByteBuffer)
-			continue;
-
-		// check for bytes sequence match
-		bool bSequenceMatch = true;
-		for (std::size_t i = 1U; i < nByteCount; i++)
-		{
-			// compare sequence and continue on wildcard or skip forward on first mismatched byte
-			if ((!bIsMaskUsed || szByteMask[i] != '?') && pCurrentByte[i] != arrByteBuffer[i])
-			{
-				// skip non suitable bytes
-				pCurrentByte += i - 1U;
-
-				bSequenceMatch = false;
-				break;
-			}
-		}
-
-		// check did we found address
-		if (bSequenceMatch)
-			vecOccurrences.push_back(pCurrentByte);
-	}
-
-	return vecOccurrences;
+std::uint8_t* MEM::FindPatternEx(const std::uint8_t* start, const std::size_t size,
+    const std::uint8_t* bytes, const std::size_t count, const char* mask)
+{
+    return const_cast<std::uint8_t*>(pattern_scan::first(start, size, bytes, count, mask));
+}
+
+std::vector<std::uint8_t*> MEM::FindPatternAllOccurrencesEx(const std::uint8_t* start,
+    const std::size_t size, const std::uint8_t* bytes, const std::size_t count, const char* mask)
+{
+    return pattern_scan::all(start, size, bytes, count, mask);
 }
 
 #pragma endregion
 
 #pragma region memory_extra
 
-std::size_t MEM::PatternToBytes(const char* szPattern, std::uint8_t* pOutByteBuffer, char* szOutMaskBuffer)
-{
-	std::uint8_t* pCurrentByte = pOutByteBuffer;
-
-	while (*szPattern != '\0')
-	{
-		// check is a wildcard
-		if (*szPattern == '?')
-		{
-			++szPattern;
-#ifdef CS_PARANOID
-			CS_ASSERT(*szPattern == '\0' || *szPattern == ' ' || *szPattern == '?'); // we're expect that next character either terminating null, whitespace or part of double wildcard (note that it's required if your pattern written without whitespaces)
-#endif
-
-			// ignore that
-			*pCurrentByte++ = 0U;
-			*szOutMaskBuffer++ = '?';
-		}
-		// check is not space
-		else if (*szPattern != ' ')
-		{
-			// convert two consistent numbers in a row to byte value
-			std::uint8_t uByte = static_cast<std::uint8_t>(CRT::CharToHexInt(*szPattern) << 4);
-
-			++szPattern;
-#ifdef CS_PARANOID
-			CS_ASSERT(*szPattern != '\0' && *szPattern != '?' && *szPattern != ' '); // we're expect that byte always represented by two numbers in a row
-#endif
-
-			uByte |= static_cast<std::uint8_t>(CRT::CharToHexInt(*szPattern));
-
-			*pCurrentByte++ = uByte;
-			*szOutMaskBuffer++ = 'x';
-		}
-
-		++szPattern;
-	}
-
-	// zero terminate both buffers
-	*pCurrentByte = 0U;
-	*szOutMaskBuffer = '\0';
-
-	return pCurrentByte - pOutByteBuffer;
+std::size_t MEM::PatternToBytes(const char* text, std::uint8_t* bytes, char* mask)
+{
+    return pattern_scan::parse(text, bytes, mask);
 }
 
 std::size_t MEM::BytesToPattern(const std::uint8_t* pByteBuffer, const std::size_t nByteCount, char* szOutBuffer)
 {
-	char* szCurrentPattern = szOutBuffer;
+    if (!szOutBuffer) return 0;
+    if (!pByteBuffer || !nByteCount) { *szOutBuffer = '\0'; return 0; }
+    char* szCurrentPattern = szOutBuffer;
 
 	for (std::size_t i = 0U; i < nByteCount; i++)
 	{
```

## `cstrike/core/sdk.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 14 | экспорт | `fnConColorMsg = reinterpret_cast<decltype(fnConColorMsg)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("?ConColorMsg@@YAXAEBVColor@@PEBDZZ")));` |

Здесь нет FindPattern, ResolveRelativeAddress, игровых offset или VTable. Только экспорт `?ConColorMsg@@YAXAEBVColor@@PEBDZZ` из tier0.dll. Существующая проверка результата корректна; наличия экспорта в локальном tier0.dll не проверяли. Код сохранён.

## `cstrike/sdk/interfaces/ccsgoinput.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 9 | индекс / предел | `#define MULTIPLAYER_BACKUP 150` |
| 14 | offset / размер / mask / layout | `float flWhen; //0x0000` |
| 15 | offset / размер / mask / layout | `MEM_PAD(0x4); //0x0004` |
| 16 | offset / размер / mask / layout | `std::uint64_t nButton; //0x0008` |
| 17 | offset / размер / mask / layout | `bool bPressed; //0x0010` |
| 18 | offset / размер / mask / layout | `MEM_PAD(0x7); //0x0011` |
| 19 | offset / размер / mask / layout | `}; //Size: 0x0018` |
| 24 | offset / размер / mask / layout | `std::uint64_t nKeyboardPressed; //0x0000` |
| 25 | offset / размер / mask / layout | `std::uint64_t nMouseWheelheelPressed; //0x0008` |
| 26 | offset / размер / mask / layout | `std::uint64_t nUnPressed; //0x0010` |
| 27 | offset / размер / mask / layout | `std::uint64_t nKeyboardCopy; //0x0018` |
| 28 | offset / размер / mask / layout | `}; //Size: 0x0020` |
| 34 | offset / размер / mask / layout | `float flForwardMove; //0x0020` |
| 35 | offset / размер / mask / layout | `float flSideMove; //0x0024` |
| 36 | offset / размер / mask / layout | `float flUpMove; //0x0028` |
| 37 | offset / размер / mask / layout | `std::int32_t nMouseDeltaX; //0x002C` |
| 38 | offset / размер / mask / layout | `std::int32_t nMouseDeltaY; //0x0030` |
| 39 | offset / размер / mask / layout | `std::int32_t nAdditionalStepMovesCount; //0x0034` |
| 40 | offset / размер / mask / layout | `CTinyMoveStepData tinyMoveStepData[12]; //0x0038` |
| 41 | offset / размер / mask / layout | `Vector_t vecViewAngle; //0x0158` |
| 42 | offset / размер / mask / layout | `std::int32_t nTargetHandle; //0x0164` |
| 43 | offset / размер / mask / layout | `}; //Size:0x0168` |
| 48 | offset / размер / mask / layout | `MEM_PAD(0x250);` |
| 50 | offset / размер / mask / layout | `MEM_PAD(0x99)` |
| 52 | offset / размер / mask / layout | `MEM_PAD(0x6);` |
| 54 | offset / размер / mask / layout | `MEM_PAD(0xE);` |
| 59 | offset / размер / mask / layout | `MEM_PAD(0x1C4);` |
| 61 | offset / размер / mask / layout | `MEM_PAD(0x48);` |
| 75 | IDA pattern | `static auto oSetViewAngle = reinterpret_cast<fnSetViewAngle>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 75 3F 48")));` |
| 87 | IDA pattern | `static auto oGetViewAngles = reinterpret_cast<fnGetViewAngles>(MEM::FindPattern(CLIENT_DLL, CS_XOR("4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3")));` |

VTable-вызовов внутри заголовка нет. В нём два pattern-based метода и ручной layout CCSGOInput. Исправлены отрицательный modulo в GetUserCmd и вызовы null function pointers. Строка SetViewAngle длиной 5 байт (`85 D2 75 3F 48`) не включает сигнатуру полноценного пролога и обязана пройти проверку уникальности/entrypoint.

Размер 150 команд, старт +0x250 и остальные поля не подтверждены текущей игрой. Публичное dwCSGOInput даёт только глобальный адрес, не структуру. GetViewAngles описан как возврат int64 с дальнейшим cast в указатель; реальный return ABI нужно восстановить, а не менять по имени.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/ccsgoinput.h
+++ b/cstrike/sdk/interfaces/ccsgoinput.h
@@ -65,7 +65,7 @@
 
 	CUserCmd* GetUserCmd()
 	{
-		return &arrCommands[nSequenceNumber % MULTIPLAYER_BACKUP];
+		return nSequenceNumber >= 0 ? &arrCommands[nSequenceNumber % MULTIPLAYER_BACKUP] : nullptr;
 	}
 
 	void SetViewAngle(QAngle_t& angView)
@@ -78,7 +78,7 @@
 		CS_ASSERT(oSetViewAngle != nullptr);
 		#endif
 
-		oSetViewAngle(this, 0, std::ref(angView));
+		if (oSetViewAngle) oSetViewAngle(this, 0, angView);
 	}
 
 	QAngle_t GetViewAngles()
@@ -90,6 +90,8 @@
 		CS_ASSERT(oGetViewAngles != nullptr);
 		#endif
 
-		return *reinterpret_cast<QAngle_t*>(oGetViewAngles(this, 0));
+		if (!oGetViewAngles) return {};
+        const auto angles = reinterpret_cast<const QAngle_t*>(oGetViewAngles(this, 0));
+        return angles ? *angles : QAngle_t{};
 	}
 };
```

## `cstrike/sdk/interfaces/cgameentitysystem.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 8 | индекс / предел | `#define MAX_ENTITIES_IN_LIST 512` |
| 9 | offset / размер / mask / layout | `#define MAX_ENTITY_LISTS 64 // 0x3F` |
| 36 | offset / размер / mask / layout | `return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x1510);` |
| 45 | IDA pattern | `static auto GetBaseEntity = reinterpret_cast<fnGetBaseEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("81 FA ? ? ? ? 77 ? 8B C2 C1 F8 ? 83 F8 ? 77 ? 48 98 48 8B 4C C1 ? 48 85 C9 74 ? 8B C2 25 ? ? ? ? 48 6B C0 ? 48 03 C8 74 ? 8B 41 ? 25 ? ? ? ? 3B C2 75 ? 48 8B 01")));` |

Публичный снимок сообщает highestEntityIndex=0x2120, исходник читает +0x1510. Вместо фиксированной замены код извлекает u32 displacement из `FF 81 ? ? ? ? 48 85 D2` (публичный алгоритм a2x). При отсутствии/неоднозначности либо недопустимом индексе возвращается -1, не придуманное резервное смещение.

GetBaseEntity остаётся pattern-based, не VTable. Добавлена проверка диапазона индекса и null function pointer. Его длинный pattern пока не проверен на DLL. Get(handle) сравнивает только entry index и не проверяет serial у найденного объекта: повторное использование slot может вернуть другую сущность. Это отдельная нерешённая проверка идентичности.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/cgameentitysystem.h
+++ b/cstrike/sdk/interfaces/cgameentitysystem.h
@@ -33,7 +33,17 @@
 
 	int GetHighestEntityIndex()
 	{
-		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x1510);
+		// a2x/cs2-dumper: dwGameEntitySystem_highestEntityIndex; build 14197 reports 0x2120.
+        // Read the actual displacement, not a fallback copied from another build.
+        static const auto instruction = MEM::FindPattern(CLIENT_DLL, CS_XOR("FF 81 ? ? ? ? 48 85 D2"));
+        if (!instruction) return -1;
+        std::uint32_t offset = 0;
+        std::memcpy(&offset, instruction + 2, sizeof(offset));
+        const auto field = reinterpret_cast<const std::uint8_t*>(this) + offset;
+        if (!MEM::IsReadable(field, sizeof(int))) return -1;
+        int index = -1;
+        std::memcpy(&index, field, sizeof(index));
+        return (index >= -1 && index < MAX_TOTAL_ENTITIES) ? index : -1;
 	}
 
 private:
@@ -43,6 +53,7 @@
 		//	do { pEntity = GetBaseEntityByIndex(g_pGameEntitySystem, nCurrentIndex); ... }
 		using fnGetBaseEntity = void*(CS_THISCALL*)(void*, int);
 		static auto GetBaseEntity = reinterpret_cast<fnGetBaseEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("81 FA ? ? ? ? 77 ? 8B C2 C1 F8 ? 83 F8 ? 77 ? 48 98 48 8B 4C C1 ? 48 85 C9 74 ? 8B C2 25 ? ? ? ? 48 6B C0 ? 48 03 C8 74 ? 8B 41 ? 25 ? ? ? ? 3B C2 75 ? 48 8B 01")));
-		return GetBaseEntity(this, nIndex);
+		if (nIndex < 0 || nIndex >= MAX_TOTAL_ENTITIES || !GetBaseEntity) return nullptr;
+        return GetBaseEntity(this, nIndex);
 	}
 };
```

## `cstrike/sdk/interfaces/iengineclient.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 30 | VTable | `return MEM::CallVFunc<int, 34U>(this);` |
| 35 | VTable | `return MEM::CallVFunc<bool, 35U>(this);` |
| 40 | VTable | `return MEM::CallVFunc<bool, 36U>(this);` |
| 48 | VTable | `MEM::CallVFunc<void, 49U>(this, std::ref(nIndex), 0);` |
| 55 | VTable | `return MEM::CallVFunc<const char*, 56U>(this);` |
| 60 | VTable | `return MEM::CallVFunc<const char*, 57U>(this);` |
| 65 | VTable | `return MEM::CallVFunc<const char*, 84U>(this);` |

Слоты: 34 GetMaxClients, 35 IsInGame, 36 IsConnected, 49 GetLocalPlayer, 56/57 имена карты, 84 ProductVersion. Для 49 передаётся указатель на int и дополнительный 0, затем прибавляется 1. Ни номера, ни соглашение slot/index не установлены для локальной сборки. Новые числа не подставлены.

## `cstrike/sdk/interfaces/ienginecvar.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 101 | offset / размер / mask / layout | `const char* szName; // 0x0000` |
| 102 | offset / размер / mask / layout | `CConVar* m_pNext; // 0x0008` |
| 103 | offset / размер / mask / layout | `MEM_PAD(0x10); // 0x0010` |
| 104 | offset / размер / mask / layout | `const char* szDescription; // 0x0020` |
| 105 | offset / размер / mask / layout | `uint32_t nType; // 0x28` |
| 106 | offset / размер / mask / layout | `uint32_t nRegistered; // 0x2C` |
| 107 | offset / размер / mask / layout | `uint32_t nFlags; // 0x30` |
| 108 | offset / размер / mask / layout | `MEM_PAD(0x8); // 0x34` |
| 110 | offset / размер / mask / layout | `CVValue_t value; // 0x40` |
| 116 | offset / размер / mask / layout | `MEM_PAD(0x40);` |

VTable-вызовов нет: это прямой обход CUtlLinkedList по this+0x40. CConVar: name+0, next+8, description+0x20, type+0x28, registered+0x2C, flags+0x30, value+0x40 с учётом выравнивания. Capture VEngineCvar007 не подтверждает этот layout. Исправлены использование глобального I::Cvar вместо this и null szName; raw list layout оставлен неподтверждённым.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/ienginecvar.h
+++ b/cstrike/sdk/interfaces/ienginecvar.h
@@ -118,10 +118,10 @@
 
 	CConVar* Find(FNV1A_t uHashedName)
 	{
-		for (int i = I::Cvar->listConvars.Head(); i != I::Cvar->listConvars.InvalidIndex(); i = I::Cvar->listConvars.Next(i))
+		for (int i = listConvars.Head(); i != listConvars.InvalidIndex(); i = listConvars.Next(i))
 		{
-			CConVar* pConVar = I::Cvar->listConvars.Element(i);
-			if (pConVar == nullptr)
+			CConVar* pConVar = listConvars.Element(i);
+			if (pConVar == nullptr || pConVar->szName == nullptr)
 				continue;
 
 			if (FNV1A::Hash(pConVar->szName) == uHashedName)
@@ -134,10 +134,10 @@
 
 	void UnlockHiddenCVars()
 	{
-		for (int i = I::Cvar->listConvars.Head(); i != I::Cvar->listConvars.InvalidIndex(); i = I::Cvar->listConvars.Next(i))
+		for (int i = listConvars.Head(); i != listConvars.InvalidIndex(); i = listConvars.Next(i))
 		{
-			CConVar* pConVar = I::Cvar->listConvars.Element(i);
-			if (pConVar == nullptr)
+			CConVar* pConVar = listConvars.Element(i);
+			if (pConVar == nullptr || pConVar->szName == nullptr)
 				continue;
 
 			if (pConVar->nFlags & FCVAR_HIDDEN)
```

## `cstrike/sdk/interfaces/imaterialsystem.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 37 | offset / размер / mask / layout | `uKey = bShouldFindKey ? FindKey(szName) : 0x0;` |
| 43 | IDA pattern | `static auto oFindKeyVar = reinterpret_cast<fnFindKeyVar>(MEM::FindPattern(PARTICLES_DLL, CS_XOR("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA")));` |
| 50 | offset / размер / mask / layout | `return oFindKeyVar(szName, 0x12, 0x31415926);` |
| 56 | offset / размер / mask / layout | `MEM_PAD(0xB0);` |
| 62 | offset / размер / mask / layout | `MEM_PAD(0xB8);` |
| 74 | IDA pattern | `static auto oSetMaterialShaderType = reinterpret_cast<fnSetMaterialShaderType>(MEM::FindPattern(PARTICLES_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 4D 8B E0 4C 8B 72 ? 48 8B F9 C0 E8 ? 24 ? 3C ? 74 ? 41 B0 ? B2 ? E8 ? ? ? ? 0F B6 07 33 DB C0 E8 ? 24 ? 3C ? 75 ? 48 8B 77 ? EB ? 48 8B F3 4C 8D 44 24 ? C7 44 24 ? ? ? ? ? 48 8D 54 24 ? 89 6C 24 ? 48 8B CE 4C 89 74 24 ? E8 ? ? ? ? 8B D0 83 F8 ? 75 ? 45 33 C9 89 6C 24 ? 4C 8D 44 24 ? 4C 89 74 24 ? 48 8B D7 48 8B CE E8 ? ? ? ? 8B D0 0F B6 0F C0 E9 ? 80 E1 ? 80 F9 ? 75 ? 48 8B 4F ? EB ? 48 8B CB 8B 41 ? 85 C0 74 ? 48 8D 59 ? 83 F8 ? 76 ? 48 8B 1B 48 63 C2 4D 85 E4")));` |
| 80 | offset / размер / mask / layout | `MaterialKeyVar_t shaderVar(0x162C1777, CS_XOR("shader"));` |
| 81 | offset / размер / mask / layout | `oSetMaterialShaderType(this, shaderVar, szShaderName, 0x18);` |
| 87 | IDA pattern | `static auto oSetMaterialFunction = reinterpret_cast<fnSetMaterialFunction>(MEM::FindPattern(PARTICLES_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 0F B6 01 45 0F B6 F9 8B 2A 48 8B F9")));` |
| 94 | offset / размер / mask / layout | `oSetMaterialFunction(this, functionVar, nValue, 0x18);` |
| 99 | offset / размер / mask / layout | `MEM_PAD(0x18); // 0x0` |
| 100 | offset / размер / mask / layout | `CSceneAnimatableObject* pSceneAnimatableObject; // 0x18` |
| 101 | offset / размер / mask / layout | `CMaterial2* pMaterial; // 0x20` |
| 102 | offset / размер / mask / layout | `CMaterial2* pMaterialCpy; // 0x28` |
| 103 | offset / размер / mask / layout | `MEM_PAD(0x10);` |
| 105 | offset / размер / mask / layout | `MEM_PAD(0x8);` |
| 114 | VTable | `return MEM::CallVFunc<CMaterial2***, 14U>(this, pOutMaterial, szMaterialName);` |
| 119 | VTable | `return MEM::CallVFunc<CMaterial2**, 29U>(this, pOutMaterial, szMaterialName, pData, 0, 0, 0, 0, 0, 1);` |
| 124 | VTable | `return MEM::CallVFunc<void, 37U>(this, pInMaterial, pData);` |

IMaterialSystem2: слоты 14/29/37; CMaterial2: неявные pure-virtual слоты 0/1. Здесь ещё три паттерна particles.dll, которые нельзя исключать из ревизии материалов.

CMeshData — фактическое имя структуры: scene object+0x18, material+0x20, material copy+0x28, object info+0x40, color+0x50, размер по объявлению x64 0x58. CSceneAnimatableObject::hOwner+0xB8 и CObjectInfo::nId+0xB0 тоже ручные. Называть это подтверждённым CMeshDrawPrimitive_t нельзя. Слоты и native layout сохранены до проверки.

## `cstrike/sdk/interfaces/ischemasystem.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 10 | offset / размер / mask / layout | `#define SCHEMASYSTEM_TYPE_SCOPES_OFFSET 0x188` |
| 11 | offset / размер / mask / layout | `#define SCHEMASYSTEMTYPESCOPE_OFF1 0x3F8` |
| 12 | offset / размер / mask / layout | `#define SCHEMASYSTEMTYPESCOPE_OFF2 0x8` |
| 37 | VTable | `return MEM::CallVFunc<bool, 3U>(this, pOutSize, unkPtr);` |
| 48 | offset / размер / mask / layout | `void* pVtable; // 0x0000` |
| 49 | offset / размер / mask / layout | `const char* szName; // 0x0008` |
| 51 | offset / размер / mask / layout | `CSchemaSystemTypeScope* pSystemTypeScope; // 0x0010` |
| 52 | offset / размер / mask / layout | `uint8_t nTypeCategory; // ETypeCategory 0x0018` |
| 53 | offset / размер / mask / layout | `uint8_t nAatomicCategory; // EAtomicCategory 0x0019` |
| 58 | offset / размер / mask / layout | `SchemaString_t szName; // 0x0000` |
| 59 | offset / размер / mask / layout | `CSchemaType* pSchemaType; // 0x0008` |
| 60 | offset / размер / mask / layout | `std::uint32_t nSingleInheritanceOffset; // 0x0010` |
| 61 | offset / размер / mask / layout | `std::int32_t nMetadataSize; // 0x0014` |
| 62 | offset / размер / mask / layout | `SchemaMetadataEntryData_t* pMetaData; // 0x0018` |
| 76 | offset / размер / mask / layout | `void* pVtable; // 0x0000` |
| 78 | offset / размер / mask / layout | `const char* szName; // 0x0008` |
| 79 | offset / размер / mask / layout | `char* szDescription; // 0x0010` |
| 81 | offset / размер / mask / layout | `int m_nSize; // 0x0018` |
| 82 | offset / размер / mask / layout | `std::int16_t nFieldSize; // 0x001C` |
| 83 | offset / размер / mask / layout | `std::int16_t nStaticSize; // 0x001E` |
| 84 | offset / размер / mask / layout | `std::int16_t nMetadataSize; // 0x0020` |
| 86 | offset / размер / mask / layout | `std::uint8_t nAlignOf; // 0x0022` |
| 87 | offset / размер / mask / layout | `std::uint8_t nBaseClassesCount; // 0x0023` |
| 88 | offset / размер / mask / layout | `char pad2[0x4]; // 0x0024` |
| 89 | offset / размер / mask / layout | `SchemaClassFieldData_t* pFields; // 0x0028` |
| 90 | offset / размер / mask / layout | `char pad3[0x8]; // 0x0030` |
| 91 | offset / размер / mask / layout | `SchemaBaseClassInfoData_t* pBaseClasses; // 0x0038` |
| 92 | offset / размер / mask / layout | `char pad4[0x28]; // 0x0040` |
| 95 | комментарий | `//SchemaClassFieldData_t* pFields; // 0x0028` |
| 127 | offset / размер / mask / layout | `MEM_PAD(0x10); // 0x0010` |
| 146 | offset / размер / mask / layout | `char* szBindingName_; // 0x0008` |
| 147 | offset / размер / mask / layout | `char* szDllName_; // 0x0010` |
| 148 | offset / размер / mask / layout | `std::int8_t nAlign_; // 0x0018` |
| 149 | offset / размер / mask / layout | `MEM_PAD(0x3); // 0x0019` |
| 150 | offset / размер / mask / layout | `std::int16_t nSize_; // 0x001C` |
| 151 | offset / размер / mask / layout | `std::int16_t nFlags_; // 0x001E` |
| 153 | offset / размер / mask / layout | `MEM_PAD(0x8); // 0x0028` |
| 154 | offset / размер / mask / layout | `CSchemaSystemTypeScope* pTypeScope_; // 0x0030` |
| 155 | offset / размер / mask / layout | `MEM_PAD(0x8); // 0x0038` |
| 156 | offset / размер / mask / layout | `std::int32_t unk1_; // 0x0040` |
| 164 | VTable | `return MEM::CallVFunc<void, 2U>(this, pReturnClass, szClassName);` |
| 169 | VTable | `return MEM::CallVFunc<CSchemaType*, 4U>(this, szName, pSchema);` |
| 174 | VTable | `return MEM::CallVFunc<CSchemaType*, 5U>(this, szName);` |
| 179 | VTable | `return MEM::CallVFunc<CSchemaType*, 6U>(this, szName);` |
| 184 | VTable | `return MEM::CallVFunc<CSchemaClassBinding*, 7U>(this, szName);` |
| 187 | offset / размер / mask / layout | `void* pVtable; // 0x0000` |
| 188 | offset / размер / mask / layout | `char szName[256U]; // 0x0008` |
| 189 | offset / размер / mask / layout | `MEM_PAD(SCHEMASYSTEMTYPESCOPE_OFF1); // 0x0108` |
| 190 | offset / размер / mask / layout | `CUtlTSHash<CSchemaClassBinding*, 256, unsigned int> hashClasses; // 0x0588` |
| 191 | offset / размер / mask / layout | `MEM_PAD(SCHEMASYSTEMTYPESCOPE_OFF2); // 0x05C8` |
| 192 | offset / размер / mask / layout | `CUtlTSHash<CSchemaEnumBinding*, 256, unsigned int> hashEnumes; // 0x2DD0` |
| 200 | VTable | `return MEM::CallVFunc<CSchemaSystemTypeScope*, 13U>(this, m_module_name, nullptr);` |
| 204 | offset / размер / mask / layout | `MEM_PAD(SCHEMASYSTEM_TYPE_SCOPES_OFFSET); // 0x0000` |

CSchemaType::GetSizes=3; scope FindDeclaredClass=2, FindSchemaTypeByName=4, FindTypeDeclaredClass=5, FindTypeDeclaredEnum=6, FindRawClassBinding=7; ISchemaSystem::FindTypeScopeForModule=13. Все эти номера требуют binary-level подтверждения.

Ручные offsets: ISchemaSystem vecTypeScopes=0x188, scope pad=0x3F8 и 0x8. Комментарий hashClasses+0x588 противоречит объявлению: 8+256+0x3F8=0x500. Исправлен комментарий, не выдуман новый runtime offset. InheritsFrom теперь отбрасывает null/self base, но произвольные циклы повреждённой schema не валидируются.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/ischemasystem.h
+++ b/cstrike/sdk/interfaces/ischemasystem.h
@@ -104,7 +104,7 @@
 		for (int i = 0; i < nBaseClassesCount; i++)
 		{
 			auto& baseClass = pBaseClasses[i];
-			if (baseClass.pClass->InheritsFrom(pClassInfo))
+			if (baseClass.pClass && baseClass.pClass != this && baseClass.pClass->InheritsFrom(pClassInfo))
 				return true;
 		}
 
@@ -187,7 +187,7 @@
 	void* pVtable; // 0x0000
 	char szName[256U]; // 0x0008
 	MEM_PAD(SCHEMASYSTEMTYPESCOPE_OFF1); // 0x0108
-	CUtlTSHash<CSchemaClassBinding*, 256, unsigned int> hashClasses; // 0x0588
+	CUtlTSHash<CSchemaClassBinding*, 256, unsigned int> hashClasses; // 0x500 for this declaration; old 0x588 comment was inconsistent
 	MEM_PAD(SCHEMASYSTEMTYPESCOPE_OFF2); // 0x05C8
 	CUtlTSHash<CSchemaEnumBinding*, 256, unsigned int> hashEnumes; // 0x2DD0
 };
```

## `cstrike/sdk/interfaces/iviewrender.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 9 | offset / размер / mask / layout | `Vector_t vecOrigin; // 0x0000` |
| 10 | offset / размер / mask / layout | `QAngle_t vecAngles; // 0x000C` |
| 11 | offset / размер / mask / layout | `float flFov; // 0x0018` |
| 12 | offset / размер / mask / layout | `char pad_0x001C[0x14]; // 0x001C` |
| 13 | offset / размер / mask / layout | `ViewMatrix_t matUNK1; // 0x0030` |
| 14 | offset / размер / mask / layout | `char pad_0x0070[0x30]; // 0x0070` |
| 15 | offset / размер / mask / layout | `ViewMatrix_t matUNK2; // 0x00A0` |
| 16 | offset / размер / mask / layout | `char pad_0x00E0[0xC8]; // 0x00E0` |
| 17 | offset / размер / mask / layout | `ViewMatrix_t matUNK3; // 0x01A8` |
| 18 | offset / размер / mask / layout | `char pad_0x01E8[0x28]; // 0x01E8` |

Это plain layout без VTable: origin0, angles0xC, FOV0x18, матрицы0x30/0xA0/0x1A8, итог0x210 по объявлению. В хуке читается origin от переданного аргумента; глобального I::ViewRender нет. Снимок dwViewRender не подтверждает, что hook argument указывает на начало того же класса. Не изменено.

## `cstrike/sdk/interfaces/iglobalvars.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 9 | offset / размер / mask / layout | `float flRealTime; //0x0000` |
| 10 | offset / размер / mask / layout | `int32_t nFrameCount; //0x0004` |
| 11 | offset / размер / mask / layout | `float flFrameTime; //0x0008` |
| 12 | offset / размер / mask / layout | `float flFrameTime2; //0x000C` |
| 13 | offset / размер / mask / layout | `int32_t nMaxClients; //0x0010` |
| 14 | offset / размер / mask / layout | `MEM_PAD(0x1C);` |
| 15 | offset / размер / mask / layout | `float flFrameTime3; //0x0030` |
| 16 | offset / размер / mask / layout | `float flCurrentTime; //0x0034` |
| 17 | offset / размер / mask / layout | `float flCurrentTime2; //0x0038` |
| 18 | offset / размер / mask / layout | `MEM_PAD(0xC);` |
| 19 | offset / размер / mask / layout | `int32_t nTickCount; //0x0048` |

В архиве класс называется IGlobalVars, не CGlobalVarsBase. RealTime0, FrameCount4, FrameTime8, FrameTime2C, MaxClients10, FrameTime330, CurrentTime34, CurrentTime238, TickCount48; итог0x4C по объявлению. Подтверждение адреса глобального pointer не подтверждает эти поля. Расширение структуры произвольным padding не сделано.

## `cstrike/core/hooks.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 53 | VTable | `if (!hkPresent.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::PRESENT), reinterpret_cast<void*>(&Present)))` |
| 57 | VTable | `if (!hkResizeBuffers.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::RESIZEBUFFERS), reinterpret_cast<void*>(&ResizeBuffers)))` |
| 71 | VTable | `if (!hkCreateSwapChain.Create(MEM::GetVFunc(pIDXGIFactory, VTABLE::DXGI::CREATESWAPCHAIN), reinterpret_cast<void*>(&CreateSwapChain)))` |
| 83 | IDA pattern | `if (!hkGetMatrixForView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 81 EC ? ? ? ? 49 8B C1")), reinterpret_cast<void*>(&GetMatrixForView)))` |
| 92 | IDA pattern | `if (!hkCreateMove.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 56 57 48 8D A8")), reinterpret_cast<void*>(&CreateMove)))` |
| 96 | VTable | `if (!hkMouseInputEnabled.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::MOUSEINPUTENABLED), reinterpret_cast<void*>(&MouseInputEnabled)))` |
| 100 | VTable | `if (!hkFrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::CLIENT::FRAMESTAGENOTIFY), reinterpret_cast<void*>(&FrameStageNotify)))` |
| 109 | IDA pattern | `if (!hkLevelInit.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 56 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B F2")), reinterpret_cast<void*>(&LevelInit)))` |
| 114 | IDA pattern | `if (!hkLevelShutdown.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4 28 E9 C3 20 01 ?")), reinterpret_cast<void*>(&LevelShutdown)))` |
| 122 | комментарий | `//*(float*)(pSetup + 0x49C) = v21; // m_OrthoRight` |
| 123 | комментарий | `//*(float*)(pSetup + 0x494) = -v21; // m_OrthoLeft` |
| 124 | комментарий | `//*(float*)(pSetup + 0x498) = -v22; // m_OrthoTop` |
| 125 | комментарий | `//*(float*)(pSetup + 0x4A0) = v22; // m_OrthoBottom` |
| 126 | IDA pattern | `if (!hkOverrideView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8 20 1E ED FF")), reinterpret_cast<void*>(&OverrideView)))` |
| 132 | IDA pattern | `if (!hkDrawObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, CS_XOR("48 8B C4 48 89 50 ? 53")), reinterpret_cast<void*>(&DrawObject)))` |
| 136 | VTable | `if (!hkIsRelativeMouseMode.Create(MEM::GetVFunc(I::InputSystem, VTABLE::INPUTSYSTEM::ISRELATIVEMOUSEMODE), reinterpret_cast<void*>(&IsRelativeMouseMode)))` |
| 270 | IDA pattern | `I::GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01")), 0x3, 0x7));` |

CreateMove в исходнике: bool(CCSGOInput*, int, CUserCmd*), первый вызов оригинала с теми же аргументами. Оригинал вызывается согласованно с typedef, но актуальный машинный ABI неизвестен. Установка этого legacy hook теперь обёрнута в ASPHYXIA_ENABLE_LEGACY_USERCMD, по умолчанию выключена. Это явно неполная миграция команд, не утверждение исправленного CRC.

FrameStageNotify: void(void*, int), slot36. OverrideView: void(void*, CViewSetup*), поиск pattern; комментарий idx15 не используется. DrawObject: void с 8 аргументами, четвёртый int count, остальные pointer; pattern из scenesystem.dll. OnAddEntity/OnRemoveEntity в архиве отсутствуют, обновлять несуществующие хуки нечего. GetMatrixForView принимает 6 pointer-аргументов и возвращает ViewMatrix_t*.

Исправлены вызов Present с фактическим pSwapChain, освобождение/отвязка RTV до ResizeBuffers, проверки PVS, устаревшие SDK pointer при смене карты. Prototype CreateMove не изменён догадкой. Setup всё ещё требует ревизии COM HRESULT/rollback при частичном отказе hooks; это не закрыто заменой сигнатур.

Изменённый код (unified diff):

```diff
--- a/cstrike/core/hooks.cpp
+++ b/cstrike/core/hooks.cpp
@@ -89,9 +89,12 @@
 
 	// For now, we'll use the pattern
 	// Credit: https://www.unknowncheats.me/forum/4265695-post6331.html
-	if (!hkCreateMove.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 56 57 48 8D A8")), reinterpret_cast<void*>(&CreateMove)))
+    // FILL: verify current CreateMove call-site ABI and generated usercmd layout; see audit/ABI_REQUIRED.md.
+#if defined(ASPHYXIA_ENABLE_LEGACY_USERCMD)
+    if (!hkCreateMove.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 56 57 48 8D A8")), reinterpret_cast<void*>(&CreateMove)))
 		return false;
 	L_PRINT(LOG_INFO) << CS_XOR("\"CreateMove\" hook has been created");
+#endif
 
 	if (!hkMouseInputEnabled.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::MOUSEINPUTENABLED), reinterpret_cast<void*>(&MouseInputEnabled)))
 		return false;
@@ -162,16 +165,21 @@
 
 	F::OnPresent();
 
-	return oPresent(I::SwapChain->pDXGISwapChain, uSyncInterval, uFlags);
+	return oPresent(pSwapChain, uSyncInterval, uFlags);
 }
 
 HRESULT CS_FASTCALL H::ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
 {
 	const auto oResizeBuffer = hkResizeBuffers.GetOriginal();
 
-	auto hResult = oResizeBuffer(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
-	if (SUCCEEDED(hResult))
-		I::CreateRenderTarget();
+    const bool owned = I::SwapChain && pSwapChain == I::SwapChain->pDXGISwapChain;
+    if (owned)
+    {
+        if (I::DeviceContext) I::DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
+        I::DestroyRenderTarget();
+    }
+    auto hResult = oResizeBuffer(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
+	if (owned) I::CreateRenderTarget();
 
 	return hResult;
 }
@@ -216,7 +224,9 @@
 	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
 		return bResult;
 
-	SDK::Cmd = cmd;
+    SDK::Cmd = cmd;
+    SDK::LocalController = nullptr;
+    SDK::LocalPawn = nullptr;
 	if (SDK::Cmd == nullptr)
 		return bResult;
 
@@ -267,10 +277,10 @@
 	const auto oLevelInit = hkLevelInit.GetOriginal();
 	// if global variables are not captured during I::Setup or we join a new game, recapture it
 	if (I::GlobalVars == nullptr)
-		I::GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D 99 C7 0D 01 4C 8D 05 42 CB 0D 01")), 0x3, 0x7));
+		I::GlobalVars = MEM::ReadPointer<IGlobalVars>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 15 ? ? ? ? 48 89 42")), 0x3, 0x7));
 	
 	// disable model occlusion
-	I::PVS->Set(false);
+	if (I::PVS) I::PVS->Set(false);
 
 	return oLevelInit(pClientModeShared, szNewMap);
 }
@@ -280,6 +290,9 @@
 	const auto oLevelShutdown = hkLevelShutdown.GetOriginal();
 	// reset global variables since it got discarded by the game
 	I::GlobalVars = nullptr;
+    SDK::Cmd = nullptr;
+    SDK::LocalController = nullptr;
+    SDK::LocalPawn = nullptr;
 
 	return oLevelShutdown(pClientModeShared);
 }
```

## `cstrike/core/hooks.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 19 | индекс / предел | `PRESENT = 8U,` |
| 20 | индекс / предел | `RESIZEBUFFERS = 13U,` |
| 21 | индекс / предел | `RESIZEBUFFERS_CSTYLE = 39U,` |
| 29 | индекс / предел | `CREATESWAPCHAIN = 10U,` |
| 37 | индекс / предел | `CREATEMOVE = 21U, // Outdated (Using pattern for now)` |
| 38 | индекс / предел | `MOUSEINPUTENABLED = 19U,` |
| 39 | индекс / предел | `FRAMESTAGENOTIFY = 36U,` |
| 47 | индекс / предел | `VALID_VIEWANGLE = 7U,` |
| 55 | индекс / предел | `ISRELATIVEMOUSEMODE = 76U,` |

COM: Present8, ResizeBuffers13 и CreateSwapChain10 — известные DXGI slots. RESIZEBUFFERS_CSTYLE39 не используется; нельзя считать это альтернативным slot IDXGISwapChain::ResizeBuffers. Игровые: CREATEMOVE21 уже помечен Outdated и не используется, MouseInput19, FrameStage36, VALID_VIEWANGLE7 не используется, RelativeMouseMode76.

IsRelativeMouseMode фактически detour с bool аргументом, то есть по форме setter; имя не доказывает native return type void*. Все game slots и прототипы сохранены как неподтверждённые. OnAddEntity/OnRemoveEntity declarations нет.

## `cstrike/sdk/datatypes/usercmd.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 36 | offset / размер / mask / layout | `MOVESTEP_BITS_BUTTON = 0x1U,` |
| 37 | offset / размер / mask / layout | `MOVESTEP_BITS_PRESSED = 0x2U,` |
| 38 | offset / размер / mask / layout | `MOVESTEP_BITS_WHEN = 0x4U,` |
| 39 | offset / размер / mask / layout | `MOVESTEP_BITS_ANALOG_FORWARD_DELTA = 0x8U,` |
| 40 | offset / размер / mask / layout | `MOVESTEP_BITS_ANALOG_LEFT_DELTA = 0x10U` |
| 45 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_VIEWANGLES = 0x1U,` |
| 46 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_SHOOTPOSITION = 0x2U,` |
| 47 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_TARGETHEADPOSITIONCHECK = 0x4U,` |
| 48 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_TARGETABSPOSITIONCHECK = 0x8U,` |
| 49 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_TARGETANGCHECK = 0x10U,` |
| 50 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_CL_INTERP = 0x20U,` |
| 51 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_SV_INTERP0 = 0x40U,` |
| 52 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_SV_INTERP1 = 0x80U,` |
| 53 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_PLAYER_INTERP = 0x100U,` |
| 54 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_RENDERTICKCOUNT = 0x200U,` |
| 55 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_RENDERTICKFRACTION = 0x400U,` |
| 56 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_PLAYERTICKCOUNT = 0x800U,` |
| 57 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_PLAYERTICKFRACTION = 0x1000U,` |
| 58 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_FRAMENUMBER = 0x2000U,` |
| 59 | offset / размер / mask / layout | `INPUT_HISTORY_BITS_TARGETENTINDEX = 0x4000U` |
| 64 | offset / размер / mask / layout | `BUTTON_STATE_PB_BITS_BUTTONSTATE1 = 0x1U,` |
| 65 | offset / размер / mask / layout | `BUTTON_STATE_PB_BITS_BUTTONSTATE2 = 0x2U,` |
| 66 | offset / размер / mask / layout | `BUTTON_STATE_PB_BITS_BUTTONSTATE3 = 0x4U` |
| 71 | offset / размер / mask / layout | `BASE_BITS_MOVE_CRC = 0x1U,` |
| 72 | offset / размер / mask / layout | `BASE_BITS_BUTTONPB = 0x2U,` |
| 73 | offset / размер / mask / layout | `BASE_BITS_VIEWANGLES = 0x4U,` |
| 74 | offset / размер / mask / layout | `BASE_BITS_COMMAND_NUMBER = 0x8U,` |
| 75 | offset / размер / mask / layout | `BASE_BITS_CLIENT_TICK = 0x10U,` |
| 76 | offset / размер / mask / layout | `BASE_BITS_FORWARDMOVE = 0x20U,` |
| 77 | offset / размер / mask / layout | `BASE_BITS_LEFTMOVE = 0x40U,` |
| 78 | offset / размер / mask / layout | `BASE_BITS_UPMOVE = 0x80U,` |
| 79 | offset / размер / mask / layout | `BASE_BITS_IMPULSE = 0x100U,` |
| 80 | offset / размер / mask / layout | `BASE_BITS_WEAPON_SELECT = 0x200U,` |
| 81 | offset / размер / mask / layout | `BASE_BITS_RANDOM_SEED = 0x400U,` |
| 82 | offset / размер / mask / layout | `BASE_BITS_MOUSEDX = 0x800U,` |
| 83 | offset / размер / mask / layout | `BASE_BITS_MOUSEDY = 0x1000U,` |
| 84 | offset / размер / mask / layout | `BASE_BITS_CONSUMED_SERVER_ANGLE = 0x2000U,` |
| 85 | offset / размер / mask / layout | `BASE_BITS_CMD_FLAGS = 0x4000U,` |
| 86 | offset / размер / mask / layout | `BASE_BITS_ENTITY_HANDLE = 0x8000U` |
| 91 | offset / размер / mask / layout | `CSGOUSERCMD_BITS_BASECMD = 0x1U,` |
| 92 | offset / размер / mask / layout | `CSGOUSERCMD_BITS_LEFTHAND = 0x2U,` |
| 93 | offset / размер / mask / layout | `CSGOUSERCMD_BITS_ATTACK3START = 0x4U,` |
| 94 | offset / размер / mask / layout | `CSGOUSERCMD_BITS_ATTACK1START = 0x8U,` |
| 95 | offset / размер / mask / layout | `CSGOUSERCMD_BITS_ATTACK2START = 0x10U` |
| 118 | IDA pattern | `static auto add_to_rep_addr = reinterpret_cast<T * (__fastcall*)(RepeatedPtrField_t*, T*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 4C 8B E0 48 8B 44 24 ? 4C 8B CF"), 0x1));` |
| 128 | offset / размер / mask / layout | `MEM_PAD(0x8) // 0x0 VTABLE` |
| 129 | offset / размер / mask / layout | `std::uint32_t nHasBits; // 0x8` |
| 130 | offset / размер / mask / layout | `std::uint64_t nCachedBits; // 0xC` |
| 139 | offset / размер / mask / layout | `static_assert(sizeof(CBasePB) == 0x18);` |
| 144 | offset / размер / mask / layout | `QAngle_t angValue; // 0x18` |
| 147 | offset / размер / mask / layout | `static_assert(sizeof(CMsgQAngle) == 0x28);` |
| 152 | offset / размер / mask / layout | `Vector4D_t vecValue; // 0x18` |
| 155 | offset / размер / mask / layout | `static_assert(sizeof(CMsgVector) == 0x28);` |
| 160 | offset / размер / mask / layout | `float flFraction; // 0x18` |
| 161 | offset / размер / mask / layout | `int nSrcTick; // 0x1C` |
| 162 | offset / размер / mask / layout | `int nDstTick; // 0x20` |
| 165 | offset / размер / mask / layout | `static_assert(sizeof(CCSGOInterpolationInfoPB) == 0x28);` |
| 170 | offset / размер / mask / layout | `CMsgQAngle* pViewAngles; // 0x18` |
| 171 | offset / размер / mask / layout | `CMsgVector* pShootPosition; // 0x20` |
| 172 | offset / размер / mask / layout | `CMsgVector* pTargetHeadPositionCheck; // 0x28` |
| 173 | offset / размер / mask / layout | `CMsgVector* pTargetAbsPositionCheck; // 0x30` |
| 174 | offset / размер / mask / layout | `CMsgQAngle* pTargetAngPositionCheck; // 0x38` |
| 175 | offset / размер / mask / layout | `CCSGOInterpolationInfoPB* cl_interp; // 0x40` |
| 176 | offset / размер / mask / layout | `CCSGOInterpolationInfoPB* sv_interp0; // 0x48` |
| 177 | offset / размер / mask / layout | `CCSGOInterpolationInfoPB* sv_interp1; // 0x50` |
| 178 | offset / размер / mask / layout | `CCSGOInterpolationInfoPB* player_interp; // 0x58` |
| 179 | offset / размер / mask / layout | `int nRenderTickCount; // 0x60` |
| 180 | offset / размер / mask / layout | `float flRenderTickFraction; // 0x64` |
| 181 | offset / размер / mask / layout | `int nPlayerTickCount; // 0x68` |
| 182 | offset / размер / mask / layout | `float flPlayerTickFraction; // 0x6C` |
| 183 | offset / размер / mask / layout | `int nFrameNumber; // 0x70` |
| 184 | offset / размер / mask / layout | `int nTargetEntIndex; // 0x74` |
| 187 | offset / размер / mask / layout | `static_assert(sizeof(CCSGOInputHistoryEntryPB) == 0x78);` |
| 196 | offset / размер / mask / layout | `static_assert(sizeof(CInButtonStatePB) == 0x30);` |
| 208 | offset / размер / mask / layout | `static_assert(sizeof(CSubtickMoveStep) == 0x30);` |
| 234 | IDA pattern | `static fn_add_subtick_move_step fn_create_new_subtick_move_step = reinterpret_cast<fn_add_subtick_move_step>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 48 8B D0 48 8D 4F 18 E8 ? ? ? ? 48 8B D0"), 0x1));` |
| 247 | VTable | `return MEM::CallVFunc<int, 7U>(this);` |
| 251 | offset / размер / mask / layout | `static_assert(sizeof(CBaseUserCmdPB) == 0x80);` |
| 272 | offset / размер / mask / layout | `static_assert(sizeof(CCSGOUserCmdPB) == 0x40);` |
| 277 | offset / размер / mask / layout | `MEM_PAD(0x8) // 0x0 VTABLE` |
| 278 | offset / размер / mask / layout | `std::uint64_t nValue; // 0x8` |
| 279 | offset / размер / mask / layout | `std::uint64_t nValueChanged; // 0x10` |
| 280 | offset / размер / mask / layout | `std::uint64_t nValueScroll; // 0x18` |
| 282 | offset / размер / mask / layout | `static_assert(sizeof(CInButtonState) == 0x20);` |
| 287 | offset / размер / mask / layout | `MEM_PAD(0x8); // 0x0 VTABLE` |
| 288 | offset / размер / mask / layout | `MEM_PAD(0x10); // TODO: find out what this is, added 14.08.2024` |
| 289 | offset / размер / mask / layout | `CCSGOUserCmdPB csgoUserCmd; // 0x18` |
| 290 | offset / размер / mask / layout | `CInButtonState nButtons; // 0x58` |
| 291 | offset / размер / mask / layout | `MEM_PAD(0x20); // 0x78` |
| 314 | offset / размер / mask / layout | `static_assert(sizeof(CUserCmd) == 0x98);` |

Исправлены затеняющий template<typename T> внутри RepeatedPtrField_t<T>, negative/null/out-of-range доступы inputHistory, обход allocated capacity вместо logical count и вызовы null subtick alloc/add. Современный layout не восстановлен: изменять sizeof только чтобы прошёл static_assert нельзя.

У CBasePB nCachedBits реально +0x10 при x64 alignment, хотя комментарий говорил +0xC. Не установлено, что это cached bits: привычный generated protobuf может иметь metadata/presence/cache в другом порядке. Поэтому SetBits и битовые маски не «исправлены» простым переносом OR в другое поле.

См. отдельную таблицу layout ниже и .proto в evidence: pitch_delta/yaw_delta, prediction_offset_ticks_x256 и execution_notes отсутствуют в legacy declarations; attack3 отсутствует в текущем CSGOUserCmdPB wire schema, появились prediction FX flags. Порядок wire полей не определяет порядок C++ storage.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/datatypes/usercmd.h
+++ b/cstrike/sdk/datatypes/usercmd.h
@@ -8,6 +8,7 @@
 #include "../../core/interfaces.h"
 #include "../interfaces/imemalloc.h"
 
+// Legacy in-memory ABI retained for analysis; current .proto != C++ layout proof.
 // @source: server.dll
 enum ECommandButtons : std::uint64_t
 {
@@ -111,14 +112,13 @@
 
 	// @ida: #STR: "cl: CreateMove clamped invalid attack h" go down a bit and you will find it
 	// @ida: #STR: "cl: CreateMove - Invalid player history [ %d, %d, %.3f ] f
-	template <typename T>
 	T* add(T* element)
 	{
 		// Define the function pointer correctly
 		static auto add_to_rep_addr = reinterpret_cast<T * (__fastcall*)(RepeatedPtrField_t*, T*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 4C 8B E0 48 8B 44 24 ? 4C 8B CF"), 0x1));
 
 		// Use the function pointer to call the function
-		return add_to_rep_addr(this, element);
+		return (add_to_rep_addr && element) ? add_to_rep_addr(this, element) : nullptr;
 	}
 };
 
@@ -127,7 +127,7 @@
 public:
 	MEM_PAD(0x8) // 0x0 VTABLE
 	std::uint32_t nHasBits; // 0x8
-	std::uint64_t nCachedBits; // 0xC
+	std::uint64_t nCachedBits; // 0x10 under x64 alignment, NOT 0x0C; runtime meaning unverified
 
 	void SetBits(std::uint64_t nBits)
 	{
@@ -233,13 +233,14 @@
 		using fn_add_subtick_move_step = CSubtickMoveStep* (__fastcall*)(void*);
 		static fn_add_subtick_move_step fn_create_new_subtick_move_step = reinterpret_cast<fn_add_subtick_move_step>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 48 8B D0 48 8D 4F 18 E8 ? ? ? ? 48 8B D0"), 0x1));
 
-		if (subtickMovesField.pRep && subtickMovesField.nCurrentSize < subtickMovesField.pRep->nAllocatedSize)
+        if (subtickMovesField.nCurrentSize < 0 || subtickMovesField.nTotalSize < 0) return nullptr;
+        if (subtickMovesField.pRep && subtickMovesField.nCurrentSize < subtickMovesField.nTotalSize &&
+            subtickMovesField.nCurrentSize < subtickMovesField.pRep->nAllocatedSize)
 			return subtickMovesField.pRep->tElements[subtickMovesField.nCurrentSize++];
 
-		CSubtickMoveStep* subtick = fn_create_new_subtick_move_step(nullptr);
-		subtickMovesField.add(subtick);
-
-		return subtick;
+		if (!fn_create_new_subtick_move_step) return nullptr;
+        CSubtickMoveStep* subtick = fn_create_new_subtick_move_step(nullptr);
+        return subtick ? subtickMovesField.add(subtick) : nullptr;
 	}
 
 	int CalculateCmdCRCSize()
@@ -292,7 +293,7 @@
 
 	CCSGOInputHistoryEntryPB* GetInputHistoryEntry(int nIndex)
 	{
-		if (nIndex >= csgoUserCmd.inputHistoryField.pRep->nAllocatedSize || nIndex >= csgoUserCmd.inputHistoryField.nCurrentSize)
+		if (nIndex < 0 || !csgoUserCmd.inputHistoryField.pRep || nIndex >= csgoUserCmd.inputHistoryField.pRep->nAllocatedSize || nIndex >= csgoUserCmd.inputHistoryField.nCurrentSize || nIndex >= csgoUserCmd.inputHistoryField.nTotalSize)
 			return nullptr;
 
 		return csgoUserCmd.inputHistoryField.pRep->tElements[nIndex];
@@ -300,7 +301,7 @@
 
 	void SetSubTickAngle(const QAngle_t& angView)
 	{
-		for (int i = 0; i < this->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; i++)
+		for (int i = 0; i < this->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
 		{
 			CCSGOInputHistoryEntryPB* pInputEntry = this->GetInputHistoryEntry(i);
 			if (!pInputEntry || !pInputEntry->pViewAngles)
```

## `cstrike/features/CRC.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 60 | IDA pattern | `static const fnSerializePartialToArray oSerializePartialToArray = reinterpret_cast<fnSerializePartialToArray>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 18 55 56 57 48 81 EC 90")));` |
| 68 | offset / размер / mask / layout | `std::uintptr_t* pMessage = reinterpret_cast<uintptr_t*>(I::MemAlloc->Alloc(0x18));` |
| 70 | offset / размер / mask / layout | `auto nHasBits = static_cast<uint32_t>(pBaseCmd->nHasBits & 0xFFFFFFFFFFFFFFFC);` |
| 75 | IDA pattern | `static const fnWriteMessage oWriteMessage = reinterpret_cast<fnWriteMessage>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 10 48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 BF")));` |
| 82 | IDA pattern | `static const fnSetMessageData oSetMessageData = reinterpret_cast<fnSetMessageData>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 20 55 56 57 48 83 EC 30 49")));` |

Исправлены null-checks, проверка размера до +1, scoped-to-thread snapshot, принадлежность snapshot конкретной команде и отсутствие углов. Убрана лишняя ; после include. Добавлены проверки function pointers и результата Alloc.

Сериализатор, WriteMessage и SetMessageData по-прежнему имеют неподтверждённые ABI. CUtlBuffer передаётся by-value (0x80 байт в объявлении); нельзя автоматически заменить его на pointer без call-site. Служебный объект 0x18 байт выделяется без доказанного конструктора, strMoveCrc трактуется как std::string*, nCachedBits |= 1 и nHasBits&...FC требуют reverse generated code. CUtlBuffer не содержит деструктора — lifetime native storage не решён. Автоматический путь CRC отключён вместе с legacy CreateMove; полноценного обновления CRC не заявлено.

Изменённый код (unified diff):

```diff
--- a/cstrike/features/CRC.h
+++ b/cstrike/features/CRC.h
@@ -6,7 +6,7 @@
 // used: CUtlBuffer
 #include "../sdk/datatypes/utlbuffer.h"
 // used: CBaseUserCmdPB
-#include "../sdk/datatypes/usercmd.h";
+#include "../sdk/datatypes/usercmd.h"
 
 namespace CRC
 {
@@ -24,12 +24,19 @@
 		QAngle_t angView;
 	};
 
-	inline SavedData_t savedData;
+	inline thread_local SavedData_t savedData{};
+    inline thread_local const CBaseUserCmdPB* savedCommand = nullptr;
+    inline thread_local bool savedAngles = false;
 
 	inline void Save(CBaseUserCmdPB* pBaseCmd)
 	{
-		if (pBaseCmd->pViewAngles != nullptr)
-			savedData.angView = pBaseCmd->pViewAngles->angValue;
+        savedCommand = nullptr;
+        savedAngles = false;
+        if (!pBaseCmd || !pBaseCmd->pInButtonState) return;
+        savedCommand = pBaseCmd;
+        savedAngles = pBaseCmd->pViewAngles != nullptr;
+        if (savedAngles)
+            savedData.angView = pBaseCmd->pViewAngles->angValue;
 
 		savedData.nButtons.nValue = pBaseCmd->pInButtonState->nValue;
 		savedData.nButtons.nValueChanged = pBaseCmd->pInButtonState->nValueChanged;
@@ -38,21 +45,24 @@
 
 	inline void Apply(CUserCmd* pCmd)
 	{
-		CBaseUserCmdPB* pBaseCmd = pCmd->csgoUserCmd.pBaseCmd;
-		if (pBaseCmd == nullptr)
+        if (!pCmd) return;
+        CBaseUserCmdPB* pBaseCmd = pCmd->csgoUserCmd.pBaseCmd;
+        if (!pBaseCmd || pBaseCmd != savedCommand)
 			return;
 
 		pCmd->nButtons.nValue = savedData.nButtons.nValue;
 		pCmd->nButtons.nValueChanged = savedData.nButtons.nValueChanged;
 		pCmd->nButtons.nValueScroll = savedData.nButtons.nValueScroll;
 
-		if (pBaseCmd->pViewAngles != nullptr)
+		if (savedAngles && pBaseCmd->pViewAngles != nullptr)
 			pBaseCmd->pViewAngles->angValue = savedData.angView;
 	}
 
 	inline bool CalculateCRC(CBaseUserCmdPB* pBaseCmd)
 	{
-		int nCalcualtedCRCSize = pBaseCmd->CalculateCmdCRCSize();
+		if (!pBaseCmd || !pBaseCmd->pInButtonState || !I::MemAlloc) return false;
+        int nCalcualtedCRCSize = pBaseCmd->CalculateCmdCRCSize();
+        if (nCalcualtedCRCSize < 0 || nCalcualtedCRCSize == (std::numeric_limits<int>::max)()) return false;
 		CUtlBuffer protobufBuffer(0, 0, 0);
 		protobufBuffer.EnsureCapacity(nCalcualtedCRCSize + 1);
 
@@ -63,10 +73,12 @@
 		CS_ASSERT(oSerializePartialToArray != nullptr);
 		#endif
 
-		if (oSerializePartialToArray(pBaseCmd, protobufBuffer, nCalcualtedCRCSize))
+		if (oSerializePartialToArray && oSerializePartialToArray(pBaseCmd, protobufBuffer, nCalcualtedCRCSize))
 		{
 			std::uintptr_t* pMessage = reinterpret_cast<uintptr_t*>(I::MemAlloc->Alloc(0x18));
-			pBaseCmd->nCachedBits |= 1;
+			if (!pMessage) return false;
+            // Legacy metadata operation: requires the actual generated serializer ABI.
+            pBaseCmd->nCachedBits |= 1;
 			auto nHasBits = static_cast<uint32_t>(pBaseCmd->nHasBits & 0xFFFFFFFFFFFFFFFC);
 			if ((pBaseCmd->nHasBits & 1) != 0)
 				nHasBits = static_cast<uint32_t>(nHasBits);
@@ -85,7 +97,8 @@
 			CS_ASSERT(oSetMessageData != nullptr);
 			#endif
 
-			oWriteMessage(pMessage, protobufBuffer, nCalcualtedCRCSize);
+            if (!oWriteMessage || !oSetMessageData) { I::MemAlloc->Free(pMessage); return false; }
+            oWriteMessage(pMessage, protobufBuffer, nCalcualtedCRCSize);
 			pBaseCmd->strMoveCrc = oSetMessageData(&pBaseCmd->strMoveCrc, pMessage, &nHasBits);
 			I::MemAlloc->Free(pMessage);
 
```

## `cstrike/sdk/interfaces/cgametracemanager.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 9 | IDA pattern | `static fnGetSurfaceData oGetSurfaceData = reinterpret_cast<fnGetSurfaceData>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 85 C0 74 ? 44 38 60")), 0x1, 0x0));` |
| 43 | offset / размер / mask / layout | `m_v4 = 0x49;` |

Подтверждён copy-paste bug: pSkip2 записывался в slots[0]/[2] и collisions[0], затирая pSkip1. Исправлено на [1]/[3] и [1]. Arrays теперь получают начальные значения: invalid handle -1 и collision 0. Это исправление старой модели filter, не доказательство нового native layout/invalid sentinel.

GetSurfaceData не вызывает null function pointer. IsVisible отбрасывает all-solid и fraction>1; threshold0.97 по-прежнему эвристика, не гарантия видимости. Значения layer/m_v2=7/m_v4=0x49 не подтверждены.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/cgametracemanager.cpp
+++ b/cstrike/sdk/interfaces/cgametracemanager.cpp
@@ -12,7 +12,7 @@
 	CS_ASSERT(oGetSurfaceData != nullptr);
 #endif
 
-	return reinterpret_cast<SurfaceData_t*>(oGetSurfaceData(m_pSurface));
+	return (oGetSurfaceData && m_pSurface) ? reinterpret_cast<SurfaceData_t*>(oGetSurfaceData(m_pSurface)) : nullptr;
 }
 
 int GameTrace_t::GetHitboxId()
@@ -31,12 +31,14 @@
 
 bool GameTrace_t::IsVisible() const
 {
-	return (m_flFraction > 0.97f);
+	return !m_bAllSolid && m_flFraction > 0.97f && m_flFraction <= 1.0f;
 }
 
 TraceFilter_t::TraceFilter_t(std::uint64_t uMask, C_CSPlayerPawn* pSkip1, C_CSPlayerPawn* pSkip2, int nLayer)
 {
-	m_uTraceMask = uMask;
+    m_arrSkipHandles.fill(-1);
+    m_arrCollisions.fill(0);
+    m_uTraceMask = uMask;
 	m_v1[0] = m_v1[1] = 0;
 	m_v2 = 7;
 	m_v3 = nLayer;
@@ -52,8 +54,8 @@
 
 	if (pSkip2 != nullptr)
 	{
-		m_arrSkipHandles[0] = pSkip2->GetRefEHandle().GetEntryIndex();
-		m_arrSkipHandles[2] = pSkip2->GetOwnerHandleIndex();
-		m_arrCollisions[0] = pSkip2->GetCollisionMask();
+		m_arrSkipHandles[1] = pSkip2->GetRefEHandle().GetEntryIndex();
+		m_arrSkipHandles[3] = pSkip2->GetOwnerHandleIndex();
+		m_arrCollisions[1] = pSkip2->GetCollisionMask();
 	}
 }
```

## `cstrike/sdk/interfaces/cgametracemanager.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 16 | offset / размер / mask / layout | `MEM_PAD(0x4);` |
| 19 | offset / размер / mask / layout | `static_assert(sizeof(Ray_t) == 0x38);` |
| 24 | offset / размер / mask / layout | `MEM_PAD(0x8)` |
| 27 | offset / размер / mask / layout | `MEM_PAD(0x4)` |
| 31 | offset / размер / mask / layout | `static_assert(sizeof(SurfaceData_t) == 0x18);` |
| 36 | offset / размер / mask / layout | `MEM_PAD(0x38);` |
| 38 | offset / размер / mask / layout | `MEM_PAD(0x4);` |
| 41 | offset / размер / mask / layout | `static_assert(sizeof(TraceHitboxData_t) == 0x44);` |
| 57 | offset / размер / mask / layout | `MEM_PAD(0x38);` |
| 59 | offset / размер / mask / layout | `MEM_PAD(0x24);` |
| 64 | offset / размер / mask / layout | `MEM_PAD(0x4);` |
| 66 | offset / размер / mask / layout | `MEM_PAD(0x6);` |
| 68 | offset / размер / mask / layout | `MEM_PAD(0x4D)` |
| 69 | offset / размер / mask / layout | `}; // Size: 0x108` |
| 71 | offset / размер / mask / layout | `static_assert(sizeof(GameTrace_t) == 0x108);` |
| 76 | offset / размер / mask / layout | `MEM_PAD(0x8);` |
| 89 | offset / размер / mask / layout | `static_assert(sizeof(TraceFilter_t) == 0x40);` |
| 98 | IDA pattern | `static fnTraceShape oTraceShape = reinterpret_cast<fnTraceShape>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 20 48 89 4C 24 08 55 56 41")));` |
| 110 | IDA pattern | `static fnClipRayToEntity oClipRayToEntity = reinterpret_cast<fnClipRayToEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C")));` |

В объявлении Ray_t: start0/endC/mins18/maxs24/type34, size38. GameTrace_t: surface0/entity8/hitbox10/contents50/start78/end84/normal90/position9C/fractionAC/allSolidB6, size108. TraceFilter_t: mask8/v1=10/skipHandles20/collisions30/v2=34/v3=36/v4=37/v5=38, size40.

У TraceShape шесть аргументов, у ClipRayToEntity семь; Vector_t передаются указателями в native call. Null guard добавлен, но размер и семантика actual Ray/Filter/Trace не проверены по DLL. Сигнатуры сохранены, никаких выдуманных замен.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/interfaces/cgametracemanager.h
+++ b/cstrike/sdk/interfaces/cgametracemanager.h
@@ -101,7 +101,7 @@
 		CS_ASSERT(oTraceShape != nullptr);
 		#endif
 
-		return oTraceShape(this, pRay, &vecStart, &vecEnd, pFilter, pGameTrace);
+		return oTraceShape && pRay && pFilter && pGameTrace && oTraceShape(this, pRay, &vecStart, &vecEnd, pFilter, pGameTrace);
 	}
 
 	bool ClipRayToEntity(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, C_CSPlayerPawn* pPawn, TraceFilter_t* pFilter, GameTrace_t* pGameTrace)
@@ -113,6 +113,6 @@
 		CS_ASSERT(oClipRayToEntity != nullptr);
 		#endif
 
-		return oClipRayToEntity(this, pRay, &vecStart, &vecEnd, pPawn, pFilter, pGameTrace);
+		return oClipRayToEntity && pRay && pPawn && pFilter && pGameTrace && oClipRayToEntity(this, pRay, &vecStart, &vecEnd, pPawn, pFilter, pGameTrace);
 	}
 };
```

## `cstrike/sdk/entity.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 99 | offset / размер / mask / layout | `return GetCollision()->CollisionMask(); // Collision + 0x38` |

Здесь нет получения BoneArray и нет FindPattern/VTable. Есть вызовы GetLocalPlayer через engine/entity system и комментарий Collision+0x38; actual offset в entity.h. Проверка `this` внутри non-static метода не делает вызов через null определённым поведением C++. Код не выдаётся за обновлённое получение костей.

## `cstrike/sdk/vdata.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 16 | комментарий | `//CResourceNameTyped< CWeakHandle< InfoForResourceTypeCModel > > m_sModelName = 0x28` |
| 17 | комментарий | `//CSkillFloat m_flHeadDamageMultiplier = 0x108` |
| 18 | комментарий | `//CSkillFloat m_flChestDamageMultiplier = 0x118` |
| 19 | комментарий | `//CSkillFloat m_flStomachDamageMultiplier = 0x128` |
| 20 | комментарий | `//CSkillFloat m_flArmDamageMultiplier = 0x138` |
| 21 | комментарий | `//CSkillFloat m_flLegDamageMultiplier = 0x148` |
| 22 | комментарий | `//float32 m_flHoldBreathTime = 0x158` |
| 23 | комментарий | `//float32 m_flDrowningDamageInterval = 0x15C` |
| 24 | комментарий | `//int32 m_nDrowningDamageInitial = 0x160` |
| 25 | комментарий | `//int32 m_nDrowningDamageMax = 0x164` |
| 26 | комментарий | `//int32 m_nWaterSpeed = 0x168` |
| 27 | комментарий | `//float32 m_flUseRange = 0x16C` |
| 28 | комментарий | `//float32 m_flUseAngleTolerance = 0x170` |
| 29 | комментарий | `//float32 m_flCrouchTime = 0x174` |

Рабочие поля используют SCHEMA_ADD_FIELD, не фиксированные числа. Публичный дамп содержит m_bIsFullAuto в CCSWeaponBaseVData (+0x72D), а не CBasePlayerWeaponVData: accessor перемещён в соответствующий класс. MaxClip1 в снимке0x4D0; WeaponType0x520; Range0x830. Эти числа не вшиты вместо schema.

Старые offsets CBasePlayerVData в комментариях не исполняются. GetVData в entity.h всё ещё использует schema(m_nSubclassID)+8 и требует отдельного подтверждения.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/vdata.h
+++ b/cstrike/sdk/vdata.h
@@ -34,7 +34,6 @@
 public:
 	CS_CLASS_NO_INITIALIZER(CBasePlayerWeaponVData);
 
-	SCHEMA_ADD_FIELD(bool, IsFullAuto, "CBasePlayerWeaponVData->m_bIsFullAuto");
 	SCHEMA_ADD_FIELD(std::int32_t, GetMaxClip1, "CBasePlayerWeaponVData->m_iMaxClip1");
 };
 
@@ -43,6 +42,8 @@
 public:
 	CS_CLASS_NO_INITIALIZER(CCSWeaponBaseVData);
 
+    SCHEMA_ADD_FIELD(bool, IsFullAuto, "CCSWeaponBaseVData->m_bIsFullAuto");
+
 	SCHEMA_ADD_FIELD(std::int32_t, GetWeaponType, "CCSWeaponBaseVData->m_WeaponType");
 	SCHEMA_ADD_FIELD(float, GetRange, "CCSWeaponBaseVData->m_flRange");
 };
```

## `cstrike/features/visuals/chams.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| — | нет прямых литералов указанного типа | см. пояснение ниже |

В этом cpp нет FindPattern: CreateMaterial — указатель, полученный в memory.cpp. Исправлены обработка ошибки KV3, уникальные material names для четырёх вариантов, проверка индекса конфигурации/пустого batch, восстановление первого изменённого material/color после draw и сброс handles/bInitialized при Destroy.

Обработка всё ещё классифицирует только arrMeshDraw[0], затем передаёт весь batch. Полное исправление multi-primitive batch требует подтверждённых stride/owner layout и DrawObject ABI; оно не симулируется изменением всех элементов по неподтверждённому sizeof.

Изменённый код (unified diff):

```diff
--- a/cstrike/features/visuals/chams.cpp
+++ b/cstrike/features/visuals/chams.cpp
@@ -18,7 +18,7 @@
 CStrongHandle<CMaterial2> F::VISUALS::CHAMS::CreateMaterial(const char* szMaterialName, const char szVmatBuffer[])
 {
 	CKeyValues3* pKeyValues3 = CKeyValues3::CreateMaterialResource();
-	pKeyValues3->LoadFromBuffer(szVmatBuffer);
+	if (!pKeyValues3 || !MEM::fnCreateMaterial || !pKeyValues3->LoadFromBuffer(szVmatBuffer)) return {};
 
 	CStrongHandle<CMaterial2> pCustomMaterial = {};
 	MEM::fnCreateMaterial(nullptr, &pCustomMaterial, szMaterialName, pKeyValues3, 0, 1);
@@ -41,13 +41,13 @@
 		return bInitialized;
 
 	arrMaterials[VISUAL_MATERIAL_PRIMARY_WHITE] = CustomMaterial_t{
-		.pMaterial = CreateMaterial(CS_XOR("materials/dev/primary_white.vmat"), szVMatBufferWhiteVisible),
-		.pMaterialInvisible = CreateMaterial(CS_XOR("materials/dev/primary_white.vmat"), szVMatBufferWhiteInvisible)
+		.pMaterial = CreateMaterial(CS_XOR("materials/dev/asphyxia_white_visible.vmat"), szVMatBufferWhiteVisible),
+		.pMaterialInvisible = CreateMaterial(CS_XOR("materials/dev/asphyxia_white_hidden.vmat"), szVMatBufferWhiteInvisible)
 	};
 
 	arrMaterials[VISUAL_MATERIAL_ILLUMINATE] = CustomMaterial_t{
-		.pMaterial = CreateMaterial(CS_XOR("materials/dev/primary_white.vmat"), szVMatBufferIlluminateVisible),
-		.pMaterialInvisible = CreateMaterial(CS_XOR("materials/dev/primary_white.vmat"), szVMatBufferIlluminateInvisible)
+		.pMaterial = CreateMaterial(CS_XOR("materials/dev/asphyxia_illuminate_visible.vmat"), szVMatBufferIlluminateVisible),
+		.pMaterialInvisible = CreateMaterial(CS_XOR("materials/dev/asphyxia_illuminate_hidden.vmat"), szVMatBufferIlluminateInvisible)
 	};
 
 	bInitialized = true;
@@ -62,13 +62,16 @@
 
 void F::VISUALS::CHAMS::Destroy()
 {
+    bInitialized = false;
 	for (auto& [pVisible, pInvisible] : arrMaterials)
 	{
-		if (pVisible)
-			I::ResourceHandleUtils->DeleteResource(pVisible.pBinding);
+		if (pVisible && I::ResourceHandleUtils)
+            I::ResourceHandleUtils->DeleteResource(pVisible.pBinding);
+        pVisible = {};
 
-		if (pInvisible)
-			I::ResourceHandleUtils->DeleteResource(pInvisible.pBinding);
+		if (pInvisible && I::ResourceHandleUtils)
+            I::ResourceHandleUtils->DeleteResource(pInvisible.pBinding);
+        pInvisible = {};
 	}
 }
 
@@ -80,7 +83,7 @@
 	if (!C_GET(bool, Vars.bVisualChams))
 		return false;
 
-	if (arrMeshDraw == nullptr)
+	if (arrMeshDraw == nullptr || nDataCount <= 0)
 		return false;
 
 	if (arrMeshDraw->pSceneAnimatableObject == nullptr)
@@ -92,7 +95,7 @@
 	if (pEntity == nullptr)
 		return false;
 
-	SchemaClassInfoData_t* pClassInfo;
+	SchemaClassInfoData_t* pClassInfo = nullptr;
 	pEntity->GetSchemaClassInfo(&pClassInfo);
 	if (pClassInfo == nullptr)
 		return false;
@@ -117,7 +120,12 @@
 bool F::VISUALS::CHAMS::OverrideMaterial(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
 {
 	const auto oDrawObject = H::hkDrawObject.GetOriginal();
-	const CustomMaterial_t customMaterial = arrMaterials[C_GET(int, Vars.nVisualChamMaterial)];
+	const int selected = C_GET(int, Vars.nVisualChamMaterial);
+    if (!arrMeshDraw || nDataCount <= 0 || selected < 0 || selected >= VISUAL_MATERIAL_MAX) return false;
+    const CustomMaterial_t customMaterial = arrMaterials[selected];
+    if (!customMaterial.pMaterial || !customMaterial.pMaterialInvisible) return false;
+    const auto originalMaterial = arrMeshDraw->pMaterial;
+    const auto originalColor = arrMeshDraw->colValue;
 
 	if (C_GET(bool, Vars.bVisualChamsIgnoreZ)) // Ignore Invisible
 	{
@@ -130,5 +138,7 @@
 	arrMeshDraw->colValue = C_GET(Color_t, Vars.colVisualChams);
 	oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
 
-	return true;
+    arrMeshDraw->pMaterial = originalMaterial;
+    arrMeshDraw->colValue = originalColor;
+    return true;
 }
```

## `cstrike/sdk/datatypes/keyvalues3.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 17 | IDA pattern | `static const fnLoadKeyValues oLoadKeyValues = reinterpret_cast<fnLoadKeyValues>(MEM::GetAbsoluteAddress(MEM::FindPattern(TIER0_DLL, CS_XOR("E8 ? ? ? ? EB 36 8B 43 10")), 0x1, 0x0));` |
| 24 | offset / размер / mask / layout | `KV3ID_t kv3ID = KV3ID_t(CS_XOR("generic"), 0x41B818518343427E, 0xB5F447C23C0CDF8C);` |

Удалён поиск второй tier0 CUtlBuffer-перегрузки `E8 ? ? ? ? EB 36 8B 43 10`. LoadFromBuffer теперь bool и использует существующий точный export LoadKV3 из MEM::Setup, со строковым input и KV3ID generic. ABI этой перегрузки следует из decorated name; наличие и поведение на текущем tier0 не тестировались. Native KeyValues3 construction/lifetime остаётся отдельным нерешённым вопросом.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/datatypes/keyvalues3.cpp
+++ b/cstrike/sdk/datatypes/keyvalues3.cpp
@@ -1,26 +1,11 @@
 #include "keyvalue3.h"
-// used: CRT
-#include "../../utilities/crt.h"
-// used: utlbuffer
-#include "utlbuffer.h"
 
-void CKeyValues3::LoadFromBuffer(const char* szString)
+bool CKeyValues3::LoadFromBuffer(const char* text)
 {
-	CUtlBuffer buffer(0, (CRT::StringLength(szString) + 10), 1);
-	buffer.PutString(szString);
-	LoadKV3(&buffer);
+    if (!text || !MEM::fnLoadKV3) return false;
+    // Signature follows the decorated export already resolved by MEM::Setup:
+    // bool LoadKV3(KeyValues3*, CUtlString*, const char*, const KV3ID_t&, const char*).
+    // The const-reference parameter is represented as a pointer at the Win64 ABI boundary.
+    const KV3ID_t format{CS_XOR("generic"), 0x41B818518343427E, 0xB5F447C23C0CDF8C};
+    return MEM::fnLoadKV3(this, nullptr, text, &format, nullptr);
 }
-
-bool CKeyValues3::LoadKV3(CUtlBuffer* buffer)
-{
-	using fnLoadKeyValues = bool(CS_FASTCALL*)(CKeyValues3*, void*, CUtlBuffer*, KV3ID_t*, void*, void*, void*, void*, const char*);
-	static const fnLoadKeyValues oLoadKeyValues = reinterpret_cast<fnLoadKeyValues>(MEM::GetAbsoluteAddress(MEM::FindPattern(TIER0_DLL, CS_XOR("E8 ? ? ? ? EB 36 8B 43 10")), 0x1, 0x0));
-
-	#ifdef CS_PARANOID
-	CS_ASSERT(oLoadKeyValues != nullptr);
-	#endif
-
-	const char* szName = CS_XOR("");
-	KV3ID_t kv3ID = KV3ID_t(CS_XOR("generic"), 0x41B818518343427E, 0xB5F447C23C0CDF8C);
-	return oLoadKeyValues(this, nullptr, buffer, &kv3ID, nullptr, nullptr, nullptr, nullptr, CS_XOR(""));
-}
```

## `cstrike/utilities/memory.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 16 | offset / размер / mask / layout | `#define MEM_PAD(SIZE)										\` |
| 51 | экспорт | `[[nodiscard]] void* GetExportAddress(const void* hModuleBase, const char* szProcedureName);` |
| 64 | relative decode | `[[nodiscard]] T* GetAbsoluteAddress(T* pRelativeAddress, int nPreOffset = 0x0, int nPostOffset = 0x0)` |
| 76 | relative decode | `[[nodiscard]] CS_INLINE std::uint8_t* ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset)` |
| 87 | VTable | `[[nodiscard]] CS_INLINE T GetVFunc(const void* thisptr, std::size_t nIndex)` |

ResolveRelativeAddress раньше загружал uint32_t, поэтому отрицательные disp32 уходили на +4GiB. Теперь int32_t копируется memcpy и расширяется со знаком. GetAbsoluteAddress тоже использует byte offsets независимо от T и возвращает null для null input. ReadPointer проверяет читаемость адреса storage; GetVFunc проверяет null/storage. Это не проверка семантики vtable entry или срока жизни объекта.

Изменённый код (unified diff):

```diff
--- a/cstrike/utilities/memory.h
+++ b/cstrike/utilities/memory.h
@@ -3,6 +3,7 @@
 #include <memory>
 // used: std::vector
 #include <vector>
+#include "pattern_scan.h"
 
 #include "../common.h"
 
@@ -25,7 +26,15 @@
 
 namespace MEM
 {
-	bool Setup();
+    bool Setup();
+    [[nodiscard]] bool IsReadable(const void* address, std::size_t bytes);
+    template<class T> [[nodiscard]] T* ReadPointer(const void* address)
+    {
+        T* value = nullptr;
+        if (IsReadable(address, sizeof(value)))
+            std::memcpy(&value, address, sizeof(value));
+        return value;
+    }
 
 	/* @section: allocation */
 	// allocate a block of memory from a heap
@@ -63,10 +72,9 @@
 	template <typename T = std::uint8_t>
 	[[nodiscard]] T* GetAbsoluteAddress(T* pRelativeAddress, int nPreOffset = 0x0, int nPostOffset = 0x0)
 	{
-		pRelativeAddress += nPreOffset;
-		pRelativeAddress += sizeof(std::int32_t) + *reinterpret_cast<std::int32_t*>(pRelativeAddress);
-		pRelativeAddress += nPostOffset;
-		return pRelativeAddress;
+        return reinterpret_cast<T*>(pattern_scan::relative(
+            reinterpret_cast<const std::uint8_t*>(pRelativeAddress), nPreOffset,
+            static_cast<std::ptrdiff_t>(nPreOffset) + sizeof(std::int32_t) + nPostOffset));
 	}
 	/// resolve rip relative address
 	/// @param[in] nAddressBytes as byte for the address we want to resolve
@@ -75,10 +83,7 @@
 	/// @returns: pointer to resolved address
 	[[nodiscard]] CS_INLINE std::uint8_t* ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset)
 	{
-		std::uint32_t nRVA = *reinterpret_cast<std::uint32_t*>(nAddressBytes + nRVAOffset);
-		std::uint64_t nRIP = reinterpret_cast<std::uint64_t>(nAddressBytes) + nRIPOffset;
-
-		return reinterpret_cast<std::uint8_t*>(nRVA + nRIP);
+        return pattern_scan::relative(nAddressBytes, nRVAOffset, nRIPOffset);
 	}
 
 	/// get pointer to function of virtual-function table
@@ -86,7 +91,11 @@
 	template <typename T = void*>
 	[[nodiscard]] CS_INLINE T GetVFunc(const void* thisptr, std::size_t nIndex)
 	{
-		return (*static_cast<T* const*>(thisptr))[nIndex];
+		if (!IsReadable(thisptr, sizeof(void*))) return T{};
+        const auto table = *static_cast<T* const*>(thisptr);
+        if (!table || nIndex > (static_cast<std::size_t>(-1) / sizeof(T)) - 1 ||
+            !IsReadable(table + nIndex, sizeof(T))) return T{};
+        return table[nIndex];
 	}
 	/// call virtual function of specified class at given index
 	/// @note: reference and const reference arguments must be forwarded as pointers or wrapped with 'std::ref'/'std::cref' calls!
```

## `cstrike/core/interfaces.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 9 | версия интерфейса | `#define GAME_RESOURCE_SERVICE_CLIENT CS_XOR("GameResourceServiceClientV00")` |
| 10 | версия интерфейса | `#define SOURCE2_CLIENT CS_XOR("Source2Client00")` |
| 11 | версия интерфейса | `#define SCHEMA_SYSTEM CS_XOR("SchemaSystem_00")` |
| 12 | версия интерфейса | `#define INPUT_SYSTEM_VERSION CS_XOR("InputSystemVersion00")` |
| 13 | версия интерфейса | `#define SOURCE2_ENGINE_TO_CLIENT CS_XOR("Source2EngineToClient00")` |
| 14 | версия интерфейса | `#define ENGINE_CVAR CS_XOR("VEngineCvar00")` |
| 15 | версия интерфейса | `#define LOCALIZE CS_XOR("Localize_00")` |
| 16 | версия интерфейса | `#define NETWORK_CLIENT_SERVICE CS_XOR("NetworkClientService_00")` |
| 17 | версия интерфейса | `#define MATERIAL_SYSTEM2 CS_XOR("VMaterialSystem2_00")` |
| 18 | версия интерфейса | `#define RESOURCE_SYSTEM CS_XOR("ResourceSystem013")` |
| 19 | версия интерфейса | `#define RESOURCE_HANDLE_UTILS CS_XOR("ResourceHandleUtils001")` |

Префиксы интерфейсов заменены точными именами из публичного снимка для прямого CreateInterface. Точность имени не равна проверке VTable ABI. Tick interval0.015625 (64Hz) — отдельное предположение, не патч-зависимый offset.

Изменённый код (unified diff):

```diff
--- a/cstrike/core/interfaces.h
+++ b/cstrike/core/interfaces.h
@@ -6,15 +6,15 @@
 #include "../sdk/interfaces/iglobalvars.h"
 
 #pragma region sdk_definitons
-#define GAME_RESOURCE_SERVICE_CLIENT CS_XOR("GameResourceServiceClientV00")
-#define SOURCE2_CLIENT CS_XOR("Source2Client00")
-#define SCHEMA_SYSTEM CS_XOR("SchemaSystem_00")
-#define INPUT_SYSTEM_VERSION CS_XOR("InputSystemVersion00")
-#define SOURCE2_ENGINE_TO_CLIENT CS_XOR("Source2EngineToClient00")
-#define ENGINE_CVAR CS_XOR("VEngineCvar00")
-#define LOCALIZE CS_XOR("Localize_00")
-#define NETWORK_CLIENT_SERVICE CS_XOR("NetworkClientService_00")
-#define MATERIAL_SYSTEM2 CS_XOR("VMaterialSystem2_00")
+#define GAME_RESOURCE_SERVICE_CLIENT CS_XOR("GameResourceServiceClientV001")
+#define SOURCE2_CLIENT CS_XOR("Source2Client002")
+#define SCHEMA_SYSTEM CS_XOR("SchemaSystem_001")
+#define INPUT_SYSTEM_VERSION CS_XOR("InputSystemVersion001")
+#define SOURCE2_ENGINE_TO_CLIENT CS_XOR("Source2EngineToClient001")
+#define ENGINE_CVAR CS_XOR("VEngineCvar007")
+#define LOCALIZE CS_XOR("Localize_001")
+#define NETWORK_CLIENT_SERVICE CS_XOR("NetworkClientService_001")
+#define MATERIAL_SYSTEM2 CS_XOR("VMaterialSystem2_001")
 #define RESOURCE_SYSTEM CS_XOR("ResourceSystem013")
 #define RESOURCE_HANDLE_UTILS CS_XOR("ResourceHandleUtils001")
 
```

## `cstrike/sdk/entity.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 39 | offset / размер / mask / layout | `SCHEMA_ADD_OFFSET(std::uint32_t, GetIndex, 0x10);` |
| 61 | offset / размер / mask / layout | `CEntityInstance* pInstance; // 0x00` |
| 71 | VTable | `return MEM::CallVFunc<void, 38U>(this, pReturn);` |
| 91 | offset / размер / mask / layout | `return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);` |
| 122 | VTable | `return MEM::CallVFunc<CSkeletonInstance*, 8U>(this);` |
| 175 | offset / размер / mask / layout | `SCHEMA_ADD_FIELD_OFFSET(void*, GetVData, "C_BaseEntity->m_nSubclassID", 0x8);` |
| 224 | VTable | `MEM::CallVFunc<void, 169U>(this, &vecEyePosition);` |
| 333 | offset / размер / mask / layout | `static_assert(sizeof(CAttributeManager) == 0x8);` |
| 426 | offset / размер / mask / layout | `MEM_PAD(0x1CC); //0x0000` |
| 427 | offset / размер / mask / layout | `int nBoneCount; //0x01CC` |
| 428 | offset / размер / mask / layout | `MEM_PAD(0x18); //0x01D0` |
| 429 | offset / размер / mask / layout | `int nMask; //0x01E8` |
| 430 | offset / размер / mask / layout | `MEM_PAD(0x4); //0x01EC` |
| 431 | offset / размер / mask / layout | `Matrix2x4_t* pBoneCache; //0x01F0` |

Bone path находится здесь: scene-node slot8 -> CSkeletonInstance, затем pBoneCache+0x1F0. Прямого BoneArray в entity.cpp нет. Снимок содержит modelState+0x140, но не публикует внутренний bone-pointer. Значения 0x1CC/0x1E8/0x1F0 не заменены популярным +0x80 без доказательства.

GetSchemaClassInfo38, GetSkeletonInstance8, GetEyePosition169; identity index+0x10, collision mask+0x38, subclass+8 — всё требует проверки. Пять полей, переехавших из C_BaseModelEntity в C_BaseEntity, переведены на новые schema names по снимку. Инициализированы output pointers classInfo. Другие отсутствующие schema keys перечислены в schema-compatibility.json, не заменены похожими именами.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/entity.h
+++ b/cstrike/sdk/entity.h
@@ -130,7 +130,7 @@
 
 	[[nodiscard]] bool IsBasePlayerController()
 	{
-		SchemaClassInfoData_t* pClassInfo;
+		SchemaClassInfoData_t* pClassInfo = nullptr;
 		GetSchemaClassInfo(&pClassInfo);
 		if (pClassInfo == nullptr)
 			return false;
@@ -145,7 +145,7 @@
 		I::SchemaSystem->FindTypeScopeForModule(CS_XOR("client.dll"))->FindDeclaredClass(&pWeaponBaseClass, CS_XOR("C_CSWeaponBase"));
 
 
-		SchemaClassInfoData_t* pClassInfo;
+		SchemaClassInfoData_t* pClassInfo = nullptr;
 		GetSchemaClassInfo(&pClassInfo);
 		if (pClassInfo == nullptr)
 			return false;
@@ -185,11 +185,11 @@
 	SCHEMA_ADD_FIELD(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
 	SCHEMA_ADD_FIELD(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
 	SCHEMA_ADD_FIELD(Vector_t, GetViewOffset, "C_BaseModelEntity->m_vecViewOffset");
-	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
-	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
-	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseModelEntity->m_hOldMoveParent");
-	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
-	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseModelEntity->m_flSimulationTime");
+	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseEntity->m_flCreateTime");
+	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseEntity->m_nCreationTick");
+	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseEntity->m_hOldMoveParent");
+	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseEntity->m_flAnimTime");
+	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseEntity->m_flSimulationTime");
 };
 
 class CPlayer_ItemServices;
```

## `cstrike/sdk/datatypes/keyvalue3.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 15 | offset / размер / mask / layout | `MEM_PAD(0x100);` |
| 18 | offset / размер / mask / layout | `MEM_PAD(0x8);` |
| 26 | IDA pattern | `static const fnSetTypeKV3 oSetTypeKV3 = reinterpret_cast<fnSetTypeKV3>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 83 EC 20 48 8B 01 48 8B D9 44")));` |
| 32 | offset / размер / mask / layout | `CKeyValues3* pKeyValue = new CKeyValues3[0x10];` |

CKeyValues3 имеет padding100/uKey100/pValue108/pad8 и размер120 по объявлению; CreateMaterialResource выделяет массив0x10 экземпляров и вызывает SetTypeKV3(...,1,6). Это не доказанный native constructor. Добавлены null guard резолвера, value-initialization и bool LoadFromBuffer. Полный lifetime и типы SetTypeKV3 остаются неизвестны.

Изменённый код (unified diff):

```diff
--- a/cstrike/sdk/datatypes/keyvalue3.h
+++ b/cstrike/sdk/datatypes/keyvalue3.h
@@ -17,8 +17,7 @@
 	void* pValue;
 	MEM_PAD(0x8);
 
-	void LoadFromBuffer(const char* szString);
-	bool LoadKV3(CUtlBuffer* buffer);
+	bool LoadFromBuffer(const char* szString);
 
 	static CKeyValues3* CreateMaterialResource()
 	{
@@ -29,7 +28,9 @@
 		CS_ASSERT(oSetTypeKV3 != nullptr);
 		#endif
 
-		CKeyValues3* pKeyValue = new CKeyValues3[0x10];
+		if (!oSetTypeKV3) return nullptr;
+        // Legacy allocation shape retained; native construction/destruction still needs verification.
+        CKeyValues3* pKeyValue = new CKeyValues3[0x10]{};
 		return oSetTypeKV3(pKeyValue, 1U, 6U);
 	}
 };
```

## `cstrike/sdk/interfaces/igameresourceservice.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 11 | offset / размер / mask / layout | `MEM_PAD(0x58);` |

Прямой доступ pGameEntitySystem+0x58. Внешний dwGameEntitySystem — альтернативное получение глобального storage, но не подтверждение этого offset. Значение не заменено без бинарника.

## `cstrike/sdk/interfaces/iswapchaindx11.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 11 | offset / размер / mask / layout | `MEM_PAD(0x170);` |

Обёртка содержит pDXGISwapChain+0x170. Это не COM layout и не стабильный DXGI offset; требует подтверждения в rendersystemdx11.dll. Не изменено.

## `cstrike/sdk/interfaces/iinputsystem.h`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 12 | offset / размер / mask / layout | `return *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x4D);` |
| 18 | offset / размер / mask / layout | `return *reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(this) + 0x26A8);` |

Дополнительные зависимости hook/menu: relative-mode bool+0x4D и SDLWindow pointer+0x26A8. Новый адрес InputSystemVersion001 не подтверждает эти поля. Не изменено.

## `cstrike/core/schema.cpp`

| Строка оригинала | Категория | Исходный код / литералы |
|---|---|---|
| 20 | offset / размер / mask / layout | `FNV1A_t uHashedFieldName = 0x0ULL;` |
| 21 | offset / размер / mask / layout | `std::uint32_t uOffset = 0x0U;` |
| 130 | offset / размер / mask / layout | `std::uint32_t uOffset = 0x0;` |
| 164 | offset / размер / mask / layout | `if (uOffset == 0x0)` |

Дополнительно исправлены утечка файлового handle при отсутствии scope, утечка pElements в GetForeignOffset, неинициализированные output classInfo и накопление старых schema entries при повторной успешной Setup.

Критичное оставшееся поведение: GetOffset при отсутствии поля возвращает 0, а reference accessor использует this+0. Новые/удалённые поля поэтому нужно проверять явно, не считать offset0 валидным результатом поиска. Перепроектирование всех reference-accessors в optional API не смешано с этой ревизией.

Изменённый код (unified diff):

```diff
--- a/cstrike/core/schema.cpp
+++ b/cstrike/core/schema.cpp
@@ -46,8 +46,12 @@
 	::WriteFile(hOutFile, szTimeBuffer.Data(), szTimeBuffer.Length(), nullptr, nullptr);
 
 	CSchemaSystemTypeScope* pTypeScope = I::SchemaSystem->FindTypeScopeForModule(szModuleName);
-	if (pTypeScope == nullptr)
-		return false;
+    if (pTypeScope == nullptr)
+    {
+        ::CloseHandle(hOutFile);
+        return false;
+    }
+    vecSchemaData.clear();
 
 	const int nTableSize = pTypeScope->hashClasses.Count();
 	L_PRINT(LOG_INFO) << CS_XOR("found \"") << nTableSize << CS_XOR("\" schema classes in module");
@@ -67,7 +71,7 @@
 		if (pClassBinding == nullptr)
 			continue;
 
-		SchemaClassInfoData_t* pDeclaredClassInfo;
+		SchemaClassInfoData_t* pDeclaredClassInfo = nullptr;
 		pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);
 
 		if (pDeclaredClassInfo == nullptr)
@@ -140,7 +144,7 @@
 		if (pClassBinding == nullptr)
 			continue;
 
-		SchemaClassInfoData_t* pDeclaredClassInfo;
+		SchemaClassInfoData_t* pDeclaredClassInfo = nullptr;
 		pTypeScope->FindDeclaredClass(&pDeclaredClassInfo, pClassBinding->szBinaryName);
 
 		if (pDeclaredClassInfo == nullptr)
@@ -161,8 +165,9 @@
 		}
 	}
 
-	if (uOffset == 0x0)
-		L_PRINT(LOG_WARNING) << CS_XOR("failed to find offset for field with hash: ") << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << uHashedFieldName;
+    delete[] pElements;
+    if (uOffset == 0x0)
+        L_PRINT(LOG_WARNING) << CS_XOR("failed to find offset for field with hash: ") << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << uHashedFieldName;
 
 	return uOffset;
 }
```

## Результаты и оставшаяся работа

Проверки: **20 016 C++ assertions под ASan/UBSan**, **5 Python unittest** — пройдены. Полная MSVC-сборка и запуск в CS2 не выполнены; Zig syntax check остановлен на отсутствующем MSVC `corecrt_math.h`. См. `VALIDATION.md`.

В `ABI_REQUIRED.md` перечислены конкретные шаги извлечения недостающих значений. Не решены текущие игровые VTable, CreateMove/CRC native ABI, SwapChain/Trace/DrawObject layouts, BoneArray и lifetime KV3. Поэтому patched-копию нельзя обозначать «обновлено под текущий патч и готово к инжекту».

Новый helper `cstrike/utilities/pattern_scan.h` используется самой исправленной реализацией memory.cpp/memory.h и проверяется `tests/pattern_tests.cpp`. `scan_pe.py` — read-only проверка сигнатур по файлам DLL; результат unique не выдаётся за доказанный ABI.

```powershell
msbuild asphyxia.sln /m /p:Configuration=Release /p:Platform=x64
```
