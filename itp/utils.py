from basic_types import CompressorName, ConcatenatedCompressorGroup

from collections import namedtuple
from typing import Iterable, List, Set, Union

Configuration = namedtuple('Configuration', 'compressors horizon difference max_quanta_count sparse')


class CompressorsGroup:
    def __init__(self, compressors: Union[CompressorName, Iterable[CompressorName]]):
        if type(compressors) is str:
            self._compressors = compressors
        else:
            self._compressors = ''
            for compressor in compressors:
                self._compressors += (compressor + '_')

            # We need to remove last '_'.
            self._compressors = self._compressors[:-1]

    def as_list(self) -> List[CompressorName]:
        return List[map(CompressorName, self._compressors.split('_'))]

    def as_string(self) -> ConcatenatedCompressorGroup:
        return ConcatenatedCompressorGroup(self._compressors)

    def as_set(self) -> Set[CompressorName]:
        return set(self.as_list())
