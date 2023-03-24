#include <emscripten.h>
#include <functional>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "IndirectBuffer.h"

using namespace std;

struct DecodeImage {
  using Callback = std::function<void (int, int, IndirectBuffer)>;

  DecodeImage(const std::string &url, const Callback &callback) : _callback(callback) {
    EM_ASM_ARGS({
      var image = new Image;
      image.onload = function() {
        var width = image.width;
        var height = image.height;
        var canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;
        var context = canvas.getContext('2d');
        context.drawImage(image, 0, 0);
        var pixels = context.getImageData(0, 0, width, height);
        var handle = Module._DecodeImage_resize($0, pixels.data.length);
        Module._IB_[handle].set(new Uint8Array(pixels.data.buffer));
        Module._DecodeImage_finish($0, width, height);
      };
      image.src = Module.UTF8ToString($1);
      console.log("src: ", Module.UTF8ToString($1));
    }, this, url.c_str());
  }

  int resize(size_t size) {
    _buffer.resize(size);
    return _buffer.handleForEmscripten();
  }

  void finish(int width, int height) {
    _callback(width, height, std::move(_buffer));
  }

private:
  Callback _callback;
  IndirectBuffer _buffer;
};

EMSCRIPTEN_KEEPALIVE
extern "C" int DecodeImage_resize(DecodeImage *self, size_t size) {
  return self->resize(size);
}
EMSCRIPTEN_KEEPALIVE
extern "C" void DecodeImage_finish(DecodeImage *self, int width, int height) {
  self->finish(width, height);
}

static bool isImageOpaque(const IndirectBuffer &buffer) {
  return EM_ASM_INT({
    var array = Module._IB_[$0];
    for (var i = 3, n = array.length; i < n; i += 4) {
      if (array[i] < 255) {
        return false;
      }
    }
    return true;
  }, buffer.handleForEmscripten());
}

int bufferToInt(const IndirectBuffer &buffer) {
  int handle = buffer.handleForEmscripten();
  return EM_ASM_INT({
    const uint8Array = Module._IB_[$0];
    const int32Array = new Int32Array(uint8Array.buffer);
    return int32Array[0];
  }, handle);
}

double bufferToDouble(const IndirectBuffer &buffer) {
  int handle = buffer.handleForEmscripten();

  return EM_ASM_DOUBLE({
    const uint8Array = Module._IB_[$0];
    const float64Array = new Float64Array(uint8Array.buffer);
    return float64Array[0];
  }, handle);
}

// normal class
class Normal {
  private:
    string m_name;
    int m_age;
    double m_height;
    double m_weight;
    string m_address;
  public:
    Normal(string name, int age, double height, double weight, string address): m_name(name), m_age(age), m_height(height), m_weight(weight), m_address(address){}
    string getName() {
      return m_name;
    }
    int getAge() {
      return m_age;
    }
    double getHeight() {
      return m_height;
    }
    double getWeight() {
      return m_weight;
    }
    string getAddress() {
      return m_address;
    }

    void setName(string name) {
      m_name = name;
    }

    void setAge(int age) {
      m_age = age;
    }

    void setHeight(double height) {
      m_height = height;
    }

    void setWeight(double weight) {
      m_weight = weight;
    }

    void setAddress(string address) {
      m_address = address;
    }
};

// normal class
class NormalBuffer {
  private:
    IndirectBuffer m_name;
    IndirectBuffer m_age;
    IndirectBuffer m_height;
    IndirectBuffer m_weight;
    IndirectBuffer m_address;
  public:
    NormalBuffer(string name, int age, double height, double weight, string address) {
      m_name.resize(sizeof(name));
      m_name.set(0, (uint8_t *)name.c_str(), sizeof(name));
      m_age.resize(sizeof(age));
      m_age.set(0, (uint8_t *)&age, sizeof(age));
      m_height.resize(sizeof(height));
      m_height.set(0, (uint8_t *)&height, sizeof(height));
      m_weight.resize(sizeof(weight));
      m_weight.set(0, (uint8_t *)&weight, sizeof(weight));
      m_address.resize(sizeof(address));
      m_address.set(0, (uint8_t *)address.c_str(), sizeof(address));
    }


    // string getName() {
    //   return m_name;
    // }
    int getAge() {
      return bufferToInt(m_age);
    }
    double getHeight() {
      return bufferToDouble(m_height);
    }
    // double getWeight() {
    //   return m_weight;
    // }
    // string getAddress() {
    //   return m_address;
    // }

    // void setName(string name) {
    //   m_name = name;
    // }

    void setAge(int age) {
      m_age.set(0, (uint8_t *)&age, sizeof(age));
    }

    // void setHeight(double height) {
    //   m_height = height;
    // }

    // void setWeight(double weight) {
    //   m_weight = weight;
    // }

    // void setAddress(string address) {
    //   m_address = address;
    // }
};

static  std::vector<NormalBuffer*> arr;
static  std::vector<Normal*> arr2;

int main() {
  // static DecodeImage async("image.png", [](int width, int height, IndirectBuffer buffer) {
  //   printf("loaded %dx%d image outside main heap\n", width, height);
  //   printf("image is opaque: %d\n", isImageOpaque(buffer));
  // });
  int count = 200000;
  for (int i = 0; i < count; i++) {
    NormalBuffer *b = new NormalBuffer("Liangguangming", 30, 180.5, 70, "Shenzhen");
    arr.emplace_back(b);

    // Normal *b = new Normal("Liangguangming", -300, 180.5, 70, "Shenzhen");
    // arr2.emplace_back(b);
  }
  std::cout << "vector size: " << arr.size() << std::endl;

  NormalBuffer *first = arr[0];
  // Normal *first = arr2[0];
  cout << "first age: " << first->getAge() << endl;
  cout << "first height: " << first->getHeight() << endl;
  return 0;
}