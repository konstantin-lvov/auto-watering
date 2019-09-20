# Система автополива растений на плате Ардуино

*Система предназначена для осуществления качественного полива растений*

## Физическая часть:

- платформа Ардуино
- две аллюминиевые пластины 10х50мм (датчик влажности)
- компрессор для осуществления полива
- фито освещение
- вентилятор (устанавливается для использования в теплице)
- три управляемых ключа (мосфет транзисторы или реле)

## Возможности системы:

1. Полив и освещение включаются только в дневное время суток благодаря простой реализации настраиваемых программных часов
2. Реализована возможность отладки через Serial командами
	- "а" - запустить компрессор
	- "t" - установить время. формат чч:мм
	- "sgom" - установить время начала освещения
	- "sgon" - установить время конца освещения
	- "swel" - установить лимит влажности
	- "swai" - установить интервал полива
	- "spul" - установить длительность полива
3. Реализовано уплавление вентиляцией по средствам аппаратного ШИМа ардуино и мосфет транзистора.

## Дополнительная инфрмация:
![schema](auto_watering_bb.jpg)
