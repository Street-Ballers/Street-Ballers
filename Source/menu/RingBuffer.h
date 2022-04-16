#pragma once

template <class T>
class RingBuffer {
private:
  T* v;
  int n;
  int start;
  int end;

public:
  RingBuffer(int size);
  
  RingBuffer();
  
  void push(const T& x);

  const T& last();

  ~RingBuffer();
};
