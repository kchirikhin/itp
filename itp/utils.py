from collections import namedtuple


Configuration = namedtuple('Configuration', 'compressors horizon difference max_quanta_count sparse')


class Compressors:
    def __init__(self, compressors):
        if type(compressors) is str:
            self._compressors = compressors
        else:
            self._compressors = ''
            for compressor in compressors:
                self._compressors += (compressor + '_')

            self._compressors = self._compressors[:-1]

    def as_list(self):
        return self._compressors.split('_')

    def as_string(self):
        return self._compressors

    def as_set(self):
        return set(self.as_list())
