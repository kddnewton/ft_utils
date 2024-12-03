# Copyright (c) Meta Platforms, Inc. and affiliates.

# pyre-strict

from typing import Generic, Optional, TypeVar

K = TypeVar("K")
V = TypeVar("V")

class ConcurrentDict(Generic[K, V]):
    def __init__(self, initial_capacity: Optional[int] = ...) -> None: ...
    def __contains__(self, key: K) -> bool: ...
    def __setitem__(self, key: K, value: V) -> None: ...
    def __getitem__(self, key: V) -> Optional[V]: ...

class AtomicInt64:
    def __init__(self, value: int = ...) -> None: ...
    def set(self, value: int) -> None: ...
    def get(self) -> int: ...
    def incr(self) -> int: ...
    def decr(self) -> int: ...
    def __format__(self, format_spec: str) -> str: ...
    def __add__(self, other: object) -> int: ...
    def __sub__(self, other: object) -> int: ...
    def __mul__(self, other: object) -> int: ...
    def __floordiv__(self, other: object) -> int: ...
    def __neg__(self) -> int: ...
    def __pos__(self) -> int: ...
    def __abs__(self) -> int: ...
    def __bool__(self) -> bool: ...
    def __or__(self, other: object) -> int: ...  # pyre-fixme[15]: Inconsistent override
    def __xor__(self, other: object) -> int: ...
    def __and__(self, other: object) -> int: ...
    def __invert__(self) -> int: ...
    def __iadd__(self, other: object) -> "AtomicInt64": ...
    def __isub__(self, other: object) -> "AtomicInt64": ...
    def __imul__(self, other: object) -> "AtomicInt64": ...
    def __ifloordiv__(self, other: object) -> "AtomicInt64": ...
    def __ior__(self, other: object) -> "AtomicInt64": ...
    def __ixor__(self, other: object) -> "AtomicInt64": ...
    def __iand__(self, other: object) -> "AtomicInt64": ...
    def __int__(self) -> int: ...
    def __eq__(self, other: object) -> bool: ...
    def __ne__(self, other: object) -> bool: ...
    def __lt__(self, other: object) -> bool: ...
    def __le__(self, other: object) -> bool: ...
    def __gt__(self, other: object) -> bool: ...
    def __ge__(self, other: object) -> bool: ...

class AtomicReference(Generic[V]):
    def __init__(self, value: Optional[V]) -> None: ...
    def set(self, value: V) -> None: ...
    def get(self) -> Optional[V]: ...
    def exchange(self, value: V) -> Optional[V]: ...
    def compare_exchange(self, expected: V, value: V) -> bool: ...
