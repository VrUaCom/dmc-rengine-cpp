# Ада-Астра — черга реверсу

Оновлено: 2026-09-21. Область: канонічний `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Ця черга не замінює докази у звітах і не оголошує закритими інші гілки.

| Стан | Ділянка | Критерій наступного результату |
|---|---|---|
| Підтверджено в межах звіту | [Root expansion](dmc3-root-expansion-2026-09-15.md) | Перервані результати відновлено; метадані й граф перевірено |
| Підтверджено в межах звіту | [CRT/lifecycle](dmc3-crt-lifecycle-2026-09-15.md) | 83 складні CRT-входи; callbacks, global bindings, фізичні межі pointer-runs |
| Реалізовано, частково | [CPtxManager](dmc3-ptx-manager-state-2026-09-15.md) | 79 сценаріїв EXE/C++; залежності названі явно |
| Реалізовано, частково | [PTX loaders / pool](dmc3-ptx-payload-state-2026-09-15.md) | 79 сценаріїв EXE/C++; 3 host guards рахуються окремо |
| Реалізовано, частково | [Матеріалізація `0x1403366E0`](dmc3-ptx-materializer-2026-09-16.md) | 71 сценарій; три failure paths; різний rollback і залишок після вичерпання пулу |
| Реалізовано, частково | [Placement та pool setup](dmc3-ptx-placement-2026-09-16.md) | 134 сценарії; обидва виходи 17 conditional sites; initializer/configure, збереження payload при reset keys |
| Реалізовано, частково | [Palette helper `0x140331BD0`](dmc3-ptx-palette-2026-09-16.md) | 232 сценарії; 10/10 conditional sites; memmove, signed tables, home-slot залежність, counter/destination після rollback |
| Реалізовано, частково | [Palette context і block allocator](dmc3-ptx-palette-lifecycle-2026-09-21.md) | 310 сценаріїв; initialize/cleanup, route/search/allocate/free; 44/49 умов з обома виходами, п'ять site-ів constant-format dispatch пояснено |
| Наступна | Callback drain `0x1403292A0` та arena setup | Відновити callback dispatch/recycling, створення арен, потім caller ordering глобального `0x140CF1030` та embedded `this+0x5E0` |
| Відкрита | Graphics config `0x140D6D300` і pool lifecycle | Тип, живі значення, callers initializer/configure, ресурсне очищення перед reset keys |
| Відкрита | Відновлення після partial allocation | Context cleanup прибирає published allocation failures; непов'язані placement failures і поточна невдала текстура потребують дослідження scene/global cleanup |
| Відкрита | Finalizer `0x140331A80` | Поведінка render descriptor, зовнішні виклики, зв'язок із draw/shutdown |
| Відкрита | Parser/backend `0x1403365B0` | Докази для TM2 і descriptor+DDS без повторення вже закритого corpus framing |
| Відкрита | 899 неохоплених vtable-кандидатів | Класифікувати код/дані/дублікати/хибні кандидати; не додавати всі адреси як функції автоматично |
| Відкрита | Семантичні межі virtual tables | Підтвердити consumers і сигнатури, не прирівнювати фізичні pointer-runs до методів |
| Відкрита | CMcAppli та scene lifecycle | Шляхи initialize/update/draw/shutdown, володіння й teardown за доказами EXE |

GitHub Projects дозволено використовувати користувачем. У поточному сеансі
підключення не надало операцій Projects, тому жоден із трьох проєктів не
обрано й не змінено. Черга зберігається в гілці; її можна перенести в
відповідний існуючий Project, коли з'явиться доступ до його вмісту й полів.
