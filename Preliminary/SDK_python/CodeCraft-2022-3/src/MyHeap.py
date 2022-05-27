import heapq

class MyHeap(object):
    def __init__(self, initial=None, key=lambda x:x):
        self.k = 20       #  the Size of this Heap
        self.key = key
        self._data = []
    def push(self, item):
        if len(self._data) < self.k:
            heapq.heappush(self._data, (self.key(item), item))
        else:
            topk_small = list(self._data[0])
            if item.a > topk_small[1].a:
                heapq.heapreplace(self._data, (self.key(item), item))
    def pop(self):
        if(len(self._data)>1):
            return heapq.heappop(self._data)[1]
        else:
            return None