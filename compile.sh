emcc -g \
  test.cpp \
  IndirectBuffer.cpp \
  -I \
  IndirectBuffer.h \
  -std=c++11 \
  -s EXPORTED_RUNTIME_METHODS=["UTF8ToString"] \
  -s ALLOW_MEMORY_GROWTH=1