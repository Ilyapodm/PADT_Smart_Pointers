# PADT Smart Pointers

## Overview

## Features

## Implemented Components

### UniquePtr

### SharedPtr

## Project Structure

## Build

### Requirements

### Build Instructions

## API Overview

- Оператор `*` - разыменовывает указатель.
Оператор `->` - 

`get` возвращает сырой указатель, но не передает владение. Вызывающий код не должен вызывать `delete`

`release` - возвращает указатель и отказывается от владения. Ответственность лежит на вызывающем коде.

`reset` - корректно завершает текущее владение и принимает новое

## Design Decisions

Why don't use exceptions in UniquePtr? - UniquePtr is a **thin** wrapper of the raw pointer, so user have to use it like raw pointer (carefully) 

Новый SharedPtr из raw pointer создаёт новую группу владения. Чтобы присоединиться к существующей группе, нужно копировать сам SharedPtr.

### Template Specializations

### Ownership Model

### Deletion Strategy

`is_array()` переводит решение об удалении в runtime, а значит в одном классе будут смешаны интерфейсы `*`, `[]` и тд.

### Code Duplication vs Abstraction

### Exception Safety

## Testing

### Memory checks

Сценарии для Valgrind и ASan/UBSan находятся в [tests/memory](tests/memory/README.md).
Цели `check_valgrind` и `check_sanitizers` проверяют владение объектами и массивами,
освобождение памяти и рост `DynamicArray`. Инструкция сборки и запуска — в README
этого каталога; GoogleTest для этих целей не требуется.

### Pointer benchmarks

Бенчмарки сравнивают raw pointer, собственные и стандартные умные указатели
для 100 000, 1 000 000 и 10 000 000 объектов. Каждая итерация выделяет один
`uint64_t` и сразу освобождает его. Измеряется полный цикл выделения и
освобождения, включая создание и уничтожение умного указателя.

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-release --target pointer_benchmarks
./build-release/tests/benchmarks/pointer_benchmarks
```

Вывод содержит две таблицы времени в миллисекундах: медиану трёх запусков
после прогрева. Порядок вариантов меняется между запусками. Для `SharedPtr`
и `std::shared_ptr` дополнительно выделяется блок счётчика ссылок;
`std::shared_ptr` создаётся из `new`, чтобы сравнивать одинаковую стратегию,
без `make_shared`. Все варианты вызывают одинаковую внешнюю функцию,
которая предотвращает удаление аллокаций оптимизатором; её вызов входит во время.
Бенчмарки не измеряют копирование разделяемого владения и многопоточную работу.

Цель `pointer_benchmarks` не зависит от GoogleTest. Для отключения её сборки
используйте `-DBUILD_BENCHMARKS=OFF`.
