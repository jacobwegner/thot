from typing import Any, List, Sequence, Tuple, overload

class NGramLanguageModel:
    def __init__(self) -> None: ...
    def load(self, filename: str) -> bool: ...
    def get_sentence_log_probability(self, sentence: Sequence[str]) -> float: ...
    def clear(self) -> None: ...

class WordAlignmentMatrix:
    @overload
    def __init__(self) -> None: ...
    @overload
    def __init__(self, row_length: int, column_length: int) -> None: ...
    @property
    def row_length(self) -> int: ...
    @property
    def column_length(self) -> int: ...
    def init(self, row_length: int, column_length: int) -> None: ...
    def __getitem__(self, key: Tuple[int, int]) -> bool: ...
    def __setitem__(self, key: Tuple[int, int], value: bool) -> None: ...
    def clear(self) -> None: ...
    def reset_all(self) -> None: ...
    def set_all(self) -> None: ...
    def set(self, i: int, j: int) -> None: ...
    def put_list(self, list: List[int]) -> None: ...
    def to_list(self) -> List[int]: ...
    def transpose(self) -> None: ...
    def flip(self) -> None: ...
    def intersect(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def union(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def xor(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def add(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def subtract(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def och_symmetrize(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def priority_symmetrize(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def grow(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def grow_diag(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def grow_diag_final(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def grow_diag_final_and(self, other: WordAlignmentMatrix) -> WordAlignmentMatrix: ...
    def __eq__(self, other: object) -> bool: ...
    def to_numpy(self) -> Any: ...

__all__ = ["NGramLanguageModel", "WordAlignmentMatrix"]
