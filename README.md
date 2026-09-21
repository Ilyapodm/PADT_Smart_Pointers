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

### Template Specializations

### Ownership Model

### Deletion Strategy

`is_array()` переводит решение об удалении в runtime, а значит в одном классе будут смешаны интерфейсы `*`, `[]` и тд.

### Code Duplication vs Abstraction

### Exception Safety

## Testing